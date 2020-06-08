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

/** @brief Chip types that coorelate device tree nodes to chip data files */
static constexpr uint8_t chipTypeOcmb[4] = {0x00, 0x20, 0x0d, 0x16};
static constexpr uint8_t chipTypeProc[4] = {0x49, 0xa0, 0x0d, 0x12};

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
    char* attrType = new char[1];

    pdbg_target_get_attribute(i_trgt, "ATTR_TYPE", 1, 1, attrType);

    switch (attrType[0])
    {
        case 0x05: // PROC
            type = 0x120DA049;
            break;

        case 0x4b: // OCMB_CHIP
            type = 0x160D2000;
            break;

        default:
            trace::err("Unsupported ATTR_TYPE value: 0x%02x", attrType[0]);
            assert(0);
    }

    delete[] attrType;
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

    // Make sure the model/level list is of unique values only.
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

    // Get the active chips to be analyzed and their types.
    std::vector<libhei::Chip> chipList;
    std::vector<libhei::ChipType_t> chipTypes;
    __getActiveChips(chipList, chipTypes);

    // Initialize the isolator for all chip types.
    __initializeIsolator(chipTypes);

    // Isolate attentions.
    libhei::IsolationData isoData{};
    libhei::isolate(chipList, isoData);

    // Filter signatures to determine root cause.
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
        trace::inf("Root cause attention: %p 0x%04x%02x%02x %d",
                   root.getChip().getChip(), root.getId(), root.getInstance(),
                   root.getBit(), root.getAttnType());

        // TODO: generate log information
    }

    // All done, clean up the isolator.
    libhei::uninitialize();

    return attnFound;
}

} // namespace analyzer
