#include <assert.h>
#include <libpdbg.h>

#include <hei_main.hpp>
#include <util/trace.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace analyzer
{

/**
 * @brief send chip data file to isolator
 *
 * Read a chip data file into memory and then send it to the isolator via
 * the initialize interface.
 *
 * @param i_filePath The file path and name to read into memory
 *
 * @return Returns true if the isolator was successfully initialized with
 *         a single chip data file. Returns false otherwise.
 *
 */
void initWithFile(const char* i_filePath)
{
    // open the file and seek to the end to get length
    std::ifstream fileStream(i_filePath, std::ios::binary | std::ios::ate);

    if (!fileStream.good())
    {
        trace::err("Unable to open file: %s", i_filePath);
        assert(0);
    }
    else
    {
        // get file size based on seek position
        fileStream.seekg(0, std::ios::end);
        std::ifstream::pos_type fileSize = fileStream.tellg();

        // create a buffer large enough to hold the entire file
        std::vector<char> fileBuffer(fileSize);

        // seek to the beginning of the file
        fileStream.seekg(0, std::ios::beg);

        // read the entire file into the buffer
        fileStream.read(fileBuffer.data(), fileSize);

        // done with the file
        fileStream.close();

        // initialize the isolator with the chip data
        libhei::initialize(fileBuffer.data(), fileSize);
    }
}

//------------------------------------------------------------------------------

uint8_t __attrType(pdbg_target* i_trgt)
{
    uint8_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_TYPE", 1, 1, &attr);
    return attr;
}

uint32_t __attrFapiPos(pdbg_target* i_trgt)
{
    uint32_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_FAPI_POS", 4, 1, &attr);
    return attr;
}

//------------------------------------------------------------------------------

const char* __path(const libhei::Chip& i_chip)
{
    return pdbg_target_path((pdbg_target*)i_chip.getChip());
}

const char* __attn(libhei::AttentionType_t i_attnType)
{
    const char* str = "";
    switch (i_attnType)
    {
        case libhei::ATTN_TYPE_CHECKSTOP:
            str = "CHECKSTOP";
            break;
        case libhei::ATTN_TYPE_UNIT_CS:
            str = "UNIT_CS";
            break;
        case libhei::ATTN_TYPE_RECOVERABLE:
            str = "RECOVERABLE";
            break;
        case libhei::ATTN_TYPE_SP_ATTN:
            str = "SP_ATTN";
            break;
        case libhei::ATTN_TYPE_HOST_ATTN:
            str = "HOST_ATTN";
            break;
        default:
            trace::err("Unsupported attention type: %u", i_attnType);
            assert(0);
    }
    return str;
}

uint32_t __trgt(const libhei::Signature& i_sig)
{
    auto trgt = (pdbg_target*)i_sig.getChip().getChip();

    uint8_t type = __attrType(trgt);
    uint32_t pos = __attrFapiPos(trgt);

    // Technically, the FapiPos attribute is 32-bit, but not likely to ever go
    // over 24-bit.

    return type << 24 | (pos & 0xffffff);
}

uint32_t __sig(const libhei::Signature& i_sig)
{
    return i_sig.getId() << 16 | i_sig.getInstance() << 8 | i_sig.getBit();
}

//------------------------------------------------------------------------------

// Returns the chip model/level of the given target. Also, adds the chip
// model/level to the list of type types needed to initialize the isolator.
libhei::ChipType_t __getChipType(pdbg_target* i_trgt,
                                 std::vector<libhei::ChipType_t>& o_types)
{
    libhei::ChipType_t type;

    // START WORKAROUND
    // TODO: Will need to grab the model/level from the target attributes when
    //       they are available. For now, use ATTR_TYPE to determine which
    //       currently supported value to use supported.
    uint8_t attrType = __attrType(i_trgt);
    switch (attrType)
    {
        case 0x05: // PROC
            type = 0x120DA049;
            break;

        case 0x4b: // OCMB_CHIP
            type = 0x160D2000;
            break;

        default:
            trace::err("Unsupported ATTR_TYPE value: 0x%02x", attrType);
            assert(0);
    }
    // END WORKAROUND

    o_types.push_back(type);

    return type;
}

//------------------------------------------------------------------------------

// Gathers list of active chips to analyze. Also, returns the list of chip types
// needed to initialize the isolator.
void __getActiveChips(std::vector<libhei::Chip>& o_chips,
                      std::vector<libhei::ChipType_t>& o_types)
{
    // Iterate each processor.
    pdbg_target* procTrgt;
    pdbg_for_each_class_target("proc", procTrgt)
    {
        // Active processors only.
        if (PDBG_TARGET_ENABLED != pdbg_target_probe(procTrgt))
            continue;

        // Add the processor to the list.
        o_chips.emplace_back(procTrgt, __getChipType(procTrgt, o_types));

        // Iterate the connected OCMBs, if they exist.
        pdbg_target* ocmbTrgt;
        pdbg_for_each_target("ocmb_chip", procTrgt, ocmbTrgt)
        {
            // Active OCMBs only.
            if (PDBG_TARGET_ENABLED != pdbg_target_probe(ocmbTrgt))
                continue;

            // Add the OCMB to the list.
            o_chips.emplace_back(ocmbTrgt, __getChipType(ocmbTrgt, o_types));
        }
    }

    // For debug, trace out all of the chips found.
    for (const auto& chip : o_chips)
    {
        trace::inf("chip:%s type:0x%0" PRIx32, __path(chip), chip.getType());
    }

    // Make sure the model/level list contains unique values only.
    auto itr = std::unique(o_types.begin(), o_types.end());
    o_types.resize(std::distance(o_types.begin(), itr));
}

//------------------------------------------------------------------------------

// Initializes the isolator for each specified chip type.
void __initializeIsolator(const std::vector<libhei::ChipType_t>& i_types)
{
    // START WORKAROUND
    // TODO: The chip data will eventually come from the CHIPDATA section of the
    //       PNOR. Until that support is available, we'll use temporary chip
    //       data files.
    for (const auto& type : i_types)
    {
        switch (type)
        {
            case 0x120DA049: // PROC
                initWithFile(
                    "/usr/share/openpower-hw-diags/chip_data_proc.cdb");
                break;

            case 0x160D2000: // OCMB_CHIP
                initWithFile(
                    "/usr/share/openpower-hw-diags/chip_data_ocmb.cdb");
                break;

            default:
                trace::err("Unsupported ChipType_t value: 0x%0" PRIx32, type);
                assert(0);
        }
    }
    // END WORKAROUND
}

//------------------------------------------------------------------------------

// Takes a signature list that will be filtered and sorted. The first entry in
// the returned list will be the root cause. If the returned list is empty,
// analysis failed.
void __filterRootCause(std::vector<libhei::Signature>& io_list)
{
    // For debug, trace out the original list of signatures before filtering.
    for (const auto& sig : io_list)
    {
        trace::inf("Signature: %s 0x%0" PRIx32 " %s", __path(sig.getChip()),
                   __sig(sig), __attn(sig.getAttnType()));
    }

    // Special and host attentions are not supported by this user application.
    auto newEndItr =
        std::remove_if(io_list.begin(), io_list.end(), [&](const auto& t) {
            return (libhei::ATTN_TYPE_SP_ATTN == t.getAttnType() ||
                    libhei::ATTN_TYPE_HOST_ATTN == t.getAttnType());
        });

    // Shrink the vector, if needed.
    io_list.resize(std::distance(io_list.begin(), newEndItr));

    // START WORKAROUND
    // TODO: Filtering should be determined by the RAS Data Files provided by
    //       the host firmware via the PNOR (similar to the Chip Data Files).
    //       Until that support is available, use a rudimentary filter that
    //       first looks for any recoverable attention, then any unit checkstop,
    //       and then any system checkstop. This is built on the premise that
    //       recoverable errors could be the root cause of an system checkstop
    //       attentions. Fortunately, we just need to sort the list by the
    //       greater attention type value.
    std::sort(io_list.begin(), io_list.end(),
              [&](const auto& a, const auto& b) {
                  return a.getAttnType() > b.getAttnType();
              });
    // END WORKAROUND
}

//------------------------------------------------------------------------------

bool analyzeHardware(std::map<std::string, std::string>& o_errors)
{
    bool attnFound = false;

    trace::inf(">>> enter analyzeHardware()");

    // Get the active chips to be analyzed and their types.
    std::vector<libhei::Chip> chipList;
    std::vector<libhei::ChipType_t> chipTypes;
    __getActiveChips(chipList, chipTypes);

    // Initialize the isolator for all chip types.
    trace::inf("Initializing isolator: # of types=%u", chipTypes.size());
    __initializeIsolator(chipTypes);

    // Isolate attentions.
    trace::inf("Isolating errors: # of chips=%u", chipList.size());
    libhei::IsolationData isoData{};
    libhei::isolate(chipList, isoData);

    // Filter signatures to determine root cause. We'll need to make a copy of
    // the list so that the original list is maintained for the log.
    std::vector<libhei::Signature> sigList{isoData.getSignatureList()};
    __filterRootCause(sigList);

    if (sigList.empty())
    {
        // Don't throw an error here because it could happen for during TI
        // analysis. Attention Handler will need to determine if this is an
        // actual problem.
        trace::inf("No active attentions found");
    }
    else
    {
        attnFound = true;
        trace::inf("Active attentions found: %d", sigList.size());

        libhei::Signature root = sigList.front();
        trace::inf("Root cause attention: %p 0x%04x%02x%02x %s",
                   root.getChip().getChip(), root.getId(), root.getInstance(),
                   root.getBit(), __attn(root.getAttnType()));

        // TODO: generate log information
    }

    // All done, clean up the isolator.
    trace::inf("Uninitializing isolator");
    libhei::uninitialize();

    trace::inf("<<< exit analyzeHardware()");

    return attnFound;
}

} // namespace analyzer
