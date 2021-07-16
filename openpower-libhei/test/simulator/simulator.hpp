#pragma once

#include <hei_main.hpp>

#include <algorithm>
#include <map>
#include <vector>

#include "gtest/gtest.h"

namespace libhei
{

/**
 * @brief Contains simulated chip objects and register contents used during
 *        isolation. Also contains the expected signatures to compare after
 *        isolation.
 */
class SimulatorData
{
  private: // This class cannot be instantiated. Use getSingleton() instead.
    /** @brief Default constructor. */
    SimulatorData() = default;

    /** @brief Destructor. */
    ~SimulatorData() = default;

    /** @brief Copy constructor. */
    SimulatorData(const SimulatorData&) = delete;

    /** @brief Assignment operator. */
    SimulatorData& operator=(const SimulatorData&) = delete;

  public:
    /** @brief Provides access to a singleton instance of this object. */
    static SimulatorData& getSingleton()
    {
        static SimulatorData theSimData{};
        return theSimData;
    }

  public:
    /** The list of supported chip types for the simulator. */
    enum SimChipType
    {
        SAMPLE      = 0xdeadbeef,
        EXPLORER_11 = 0x60d20011,
        EXPLORER_20 = 0x60d20020,
        P10_10      = 0x20da0010,
        P10_20      = 0x20da0020,
    };

  private:
    /** The Chip Data file paths for each support chip type. */
    static const std::map<SimChipType, const char*> cv_chipPath;

    /** The list of configured chips used throughout a test case. */
    std::vector<Chip> iv_chipList;

    /** The list of configured chip types used throughout a test case. */
    std::vector<ChipType_t> iv_typeList;

    /** The contents of all the SCOM registers used for an iteration of
     *  isolation. */
    std::map<Chip, std::map<uint32_t, uint64_t>> iv_scomRegData;

    /** The contents of all the Indirect SCOM registers used for an iteration of
     *  isolation. */
    std::map<Chip, std::map<uint64_t, uint64_t>> iv_idScomRegData;

    /** The list of expected signatures during an iteration of isolation. */
    std::vector<Signature> iv_expSigList;

  public:
    /**
     * @brief Adds a chip to the list of configured chips. Also, calls the main
     *        initialize() API which will initialize the isolator with the Chip
     *        Data File associated with this chip.
     */
    void addChip(const Chip& i_chip);

    /** @brief Retrieve ScomReg from map and return its value */
    uint64_t getScomReg(const Chip& i_chip, uint32_t i_address)
    {
        return iv_scomRegData[i_chip][i_address];
    }

    /** @breif Retrieve idScomReg from map and return its value */
    uint64_t getIdScomReg(const Chip& i_chip, uint64_t i_address)
    {
        return iv_idScomRegData[i_chip][i_address];
    }

    /** @brief Adds a SCOM register to iv_scomRegData. */
    void addScomReg(const Chip& i_chip, uint32_t i_address, uint64_t i_value)
    {
        // First check if this entry already exists.
        auto chip_itr = iv_scomRegData.find(i_chip);
        if (iv_scomRegData.end() != chip_itr)
        {
            auto addr_itr = chip_itr->second.find(i_address);
            ASSERT_EQ(chip_itr->second.end(), addr_itr);
        }

        // Add the new entry.
        iv_scomRegData[i_chip][i_address] = i_value;
    }

    /** @brief Adds a SCOM register to iv_idScomRegData. */
    void addIdScomReg(const Chip& i_chip, uint64_t i_address, uint64_t i_value)
    {
        // First check if this entry already exists.
        auto chip_itr = iv_idScomRegData.find(i_chip);
        if (iv_idScomRegData.end() != chip_itr)
        {
            auto addr_itr = chip_itr->second.find(i_address);
            ASSERT_EQ(chip_itr->second.end(), addr_itr);
        }

        // Add the new entry.
        iv_idScomRegData[i_chip][i_address] = i_value;
    }

    /** @brief Adds a Signature to iv_expSigList. */
    void addSignature(const Signature& i_signature)
    {
        // First check if this entry already exists.
        auto itr =
            std::find(iv_expSigList.begin(), iv_expSigList.end(), i_signature);
        ASSERT_EQ(iv_expSigList.end(), itr);

        // Add the new entry.
        iv_expSigList.push_back(i_signature);
    }

    /**
     * @brief Flushes register and expected signature lists used for a single
     *        isolation.
     */
    void flushIterationData()
    {
        iv_scomRegData.clear();
        iv_idScomRegData.clear();
        iv_expSigList.clear();
    }

    /** @brief Flushes all simulation data. */
    void flushAll()
    {
        flushIterationData();
        iv_chipList.clear();
        iv_typeList.clear();
    }

    /**
     * @brief After an iteration is set up with registers and expected
     *        signatures, this is called to run the simulation and verify the
     *        expected signatures.
     */
    void endIteration();
};

} // end namespace libhei

//------------------------------------------------------------------------------

// clang-format off

// The following macros can be used to simplify commonly used function for
// simulation test cases. At the core of each test case is a Google Test (i.e.
// gtest), which will do most of the error checking. Just like in gtest, a test
// case file can contain more than one test. Also, remember that this is all C++
// code. While it not likely to be used much, you can combine these macros with
// C++ code to do more advanced test cases. For example, you can put the
// iteration macros in a loop to walk through each bit of a register.

/**
 * This is the beginning of a test case. The NAME parameter must be valid C++
 * identifier and must not contain any underscores (per gtest requirement). To
 * end the test case use END_TEST_CASE. All contents of the test case must be
 * contain in between these two macros.
 */
#define START_TEST_CASE(NAME)                                                  \
    TEST(Simulator, NAME)                                                      \
    {                                                                          \
        libhei::SimulatorData& simData =                                       \
            libhei::SimulatorData::getSingleton();                             \
        simData.flushAll();                                                    \
        libhei::ChipType_t chipType;

/**
 * Use this to configure a chip object for the test case. There should be an
 * instance of this macro for each chip required for the test case. Note that
 * this will also call libhei::initialize() for each new chip type. The CHIP
 * parameter must be valid C++ identifier because it will be used as the name of
 * the chip variable. This same identifier will be re-used in several other
 * macros.
 */
#define CHIP(CHIP, TYPE)                                                       \
    chipType = static_cast<libhei::ChipType_t>(libhei::SimulatorData::TYPE);   \
    libhei::Chip CHIP{#CHIP, chipType};                                        \
    simData.addChip(CHIP);

/**
 * Once all of the chips have been configured, there can be one or more
 * iterations defined in the test case. Use END_ITERATION to end the iteration.
 * Note that register and signature information will be reset for each
 * iteration, however, the same set of configure chips will be used for all
 * iterations within the test case.
 */
#define START_ITERATION                                                        \
    {                                                                          \
        simData.flushIterationData();

/** This will add a SCOM register to the current iteration. */
#define REG_SCOM(CHIP, ADDR, VAL)                                              \
    simData.addScomReg(CHIP, static_cast<uint32_t>(ADDR),                      \
                       static_cast<uint64_t>(VAL));

/** This will add an Indirect SCOM register to the current iteration. */
#define REG_IDSCOM(CHIP, ADDR, VAL)                                            \
    simData.addIdScomReg(CHIP, static_cast<uint64_t>(ADDR),                    \
                         static_cast<uint64_t>(VAL));

/** This will add an expected signature to the current iteration. */
#define EXP_SIG(CHIP, ID, INST, BIT, TYPE)                                     \
    simData.addSignature(libhei::Signature{                                    \
        CHIP, static_cast<libhei::RegisterId_t>(ID),                           \
        static_cast<libhei::Instance_t>(INST),                                 \
        static_cast<libhei::BitPosition_t>(BIT), libhei::ATTN_TYPE_##TYPE});

/**
 * This is the end of an iteration that began with START_ITERATION. All of the
 * register contents and expected signatures will have been stored in the
 * simulation data. So, this will call libhei::isolate() with the list of
 * configured chips. Using the register contents in the simulation data,
 * libhei::isolate() will return a list of signatures (active attentions). That
 * list will be compared against the expected list of signatures stored in the
 * simulation data for test case verification.
 *
 * You will see that there are two gtest checks for failures:
 *  - The first check will look to see if any of the previous functions to add
 *    chips, registers, or signatures to the simulation data failed.
 *  - The second check will determine if isolation completed successfully and if
 *    all expected signatures have been verified.
 * If either check fails, the test case will be aborted regardless if there are
 * additional iterations in that test case. Note that failure in a test case
 * will not have any impact on subsequent test cases. Therefore, all test cases
 * in a file will at least be attempted even if there is a failure.
 */
#define END_ITERATION                                                          \
        if (HasFailure()) { simData.flushAll(); return; }                      \
        simData.endIteration();                                                \
        if (HasFailure()) { simData.flushAll(); return; }                      \
    }

/**
 * This is the end of the test case that started with START_TEST_CASE. It will
 * call libhei::uninitialize() and clean up the simulation data.
 */
#define END_TEST_CASE                                                          \
        libhei::uninitialize();                                                \
        simData.flushAll();                                                    \
    }

// clang-format on
