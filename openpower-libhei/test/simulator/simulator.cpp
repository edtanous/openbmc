#include "simulator.hpp"

#include <fstream> // std::ifstream

namespace libhei
{

//------------------------------------------------------------------------------

// Paths are relative from the build/ directory
const std::map<SimulatorData::ChipType, const char*>
    SimulatorData::cv_chipPath = {
        {SAMPLE, "../test/simulator/chip_data/sample.cdb"},
};

//------------------------------------------------------------------------------

void SimulatorData::addChip(const Chip& i_chip)
{
    // First check if this entry already exists.
    auto itr1 = std::find(iv_chipList.begin(), iv_chipList.end(), i_chip);
    ASSERT_EQ(iv_chipList.end(), itr1);

    // Add the new entry.
    iv_chipList.push_back(i_chip);

    // Look for the file path
    auto itr2 = cv_chipPath.find(static_cast<ChipType>(i_chip.getType()));
    ASSERT_NE(cv_chipPath.end(), itr2);
    const char* path = itr2->second;

    // Open the Chip Data File
    std::ifstream cdf{path, std::ifstream::binary};
    ASSERT_TRUE(cdf.good());

    // Get the length of file
    cdf.seekg(0, cdf.end);
    size_t sz_buffer = cdf.tellg();
    cdf.seekg(0, cdf.beg);

    // Allocate memory
    char* buffer = new char[sz_buffer];

    // Read data as a block
    cdf.read(buffer, sz_buffer);

    // Close the Chip Data File
    cdf.close();

    // Initilize the chip with this Chip Data File.
    ReturnCode rc = initialize(buffer, sz_buffer);
    ASSERT_TRUE(RC_SUCCESS == rc || RC_CHIP_DATA_INITIALIZED == rc);

    // Clean up the buffer
    delete[] buffer;
}

//------------------------------------------------------------------------------

void SimulatorData::endIteration()
{
    // Start by calling libhei::isolate().
    IsolationData isoData{};
    ReturnCode rc = isolate(iv_chipList, isoData);

    // It is possible that even in a failure scenario the information in the
    // returned IsolationData would be useful.
    // TODO: Figure out where to put the data.

    // Verify if isolation completed successfully.
    ASSERT_TRUE(RC_SUCCESS == rc);

    // Get the list of signatures found in isolation.
    std::vector<Signature> givenSigList = isoData.getSignatureList();

    // Verify the expected list and given list are the same.
    ASSERT_EQ(iv_expSigList.size(), givenSigList.size());

    std::sort(iv_expSigList.begin(), iv_expSigList.end());
    std::sort(givenSigList.begin(), givenSigList.end());

    ASSERT_TRUE(std::equal(givenSigList.begin(), givenSigList.end(),
                           iv_expSigList.begin()));

    // The iteration is complete so we can flush the data.
    flushIterationData();
}

} // end namespace libhei
