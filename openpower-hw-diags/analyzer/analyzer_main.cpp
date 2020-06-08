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
bool initWithFile(const char* i_filePath)
{
    using namespace libhei;

    bool rc = true; // assume success

    // open the file and seek to the end to get length
    std::ifstream fileStream(i_filePath, std::ios::binary | std::ios::ate);

    if (!fileStream)
    {
        std::cout << "could not open file" << std::endl;
        rc = false;
    }
    else
    {
        // get file size based on seek position
        std::ifstream::pos_type fileSize = fileStream.tellg();

        // create a buffer large enough to hold the entire file
        std::vector<char> fileBuffer(fileSize);

        // seek to the beginning of the file
        fileStream.seekg(0, std::ios::beg);

        // read the entire file into the buffer
        fileStream.read(fileBuffer.data(), fileSize);

        // done with the file
        fileStream.close();

        // intialize the isolator with the chip data
        initialize(fileBuffer.data(), fileSize); // hei initialize
    }

    return rc;
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

/**
 * @brief Analyze using the hardware error isolator
 *
 * Query the hardware for each active chip that is a valid candidate for
 * error analyses. Based on the list of active chips initialize the
 * isolator with the associated chip data files. Finally request analyses
 * from the hardware error isolator and log the results.
 *
 * @param o_errors A map for storing information about erros that were
 *                 detected by the hardware error isolator.
 *
 * @return True if hardware error analyses was successful, false otherwise
 */
bool analyzeHardware(std::map<std::string, std::string>& o_errors)
{
    using namespace libhei;

    bool rc = true;

    // Get the active chips to be analyzed and their types.
    std::vector<libhei::Chip> chipList;
    std::vector<libhei::ChipType_t> chipTypes;
    __getActiveChips(chipList, chipTypes);

    IsolationData isoData{}; // data from isolato

    // TODO select chip data files based on chip types detected
    do
    {
        // TODO for now chip data files are local
        // hei initialize
        if (false ==
            initWithFile("/usr/share/openpower-hw-diags/chip_data_ocmb.cdb"))
        {
            rc = false;
            break;
        }

        // TODO for now chip data files are local
        // hei initialize
        if (false ==
            initWithFile("/usr/share/openpower-hw-diags/chip_data_proc.cdb"))
        {
            rc = false;
            break;
        }

        // hei isolate
        isolate(chipList, isoData);

        if (!(isoData.getSignatureList().empty()))
        {
            // TODO parse signature list
            int numErrors = isoData.getSignatureList().size();

            std::cout << "isolated: " << numErrors << std::endl;
        }

        // hei uninitialize
        uninitialize();

    } while (0);

    return rc;
}

} // namespace analyzer
