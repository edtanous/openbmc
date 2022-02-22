#include <fcntl.h>

#include <test/sim-hw-access.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <limits>

#include "gtest/gtest.h"

TEST(PDBG, PdbgDtsTest1)
{
    const char* perv1_fapi_pos_path  = "/proc0/pib/perv1";
    const char* perv12_fapi_pos_path = "/proc0/pib/perv12";
    const uint32_t perv1_fapi_pos    = 1;
    const uint32_t perv12_fapi_pos   = 12;

    pdbg_targets_init(nullptr);

    trace::inf("retrieving fapi pos.");
    uint32_t attr     = std::numeric_limits<uint32_t>::max();
    pdbg_target* trgt = pdbg_target_from_path(nullptr, perv1_fapi_pos_path);
    pdbg_target_get_attribute(trgt, "ATTR_FAPI_POS", 4, 1, &attr);
    trace::inf("perv1 fapi pos in DTS: %u", attr);
    EXPECT_EQ(attr, perv1_fapi_pos);

    attr = std::numeric_limits<uint32_t>::max();
    trgt = pdbg_target_from_path(nullptr, perv12_fapi_pos_path);
    pdbg_target_get_attribute(trgt, "ATTR_FAPI_POS", 4, 1, &attr);
    trace::inf("perv12 fapi pos in DTS: %u", attr);
    EXPECT_EQ(attr, perv12_fapi_pos);
}

TEST(PDBG, PdbgDtsTest2)
{
    const char* dimm0_path =
        "/proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0/mem_port0/dimm0";
    const uint32_t index    = 0;
    const uint32_t fapi_pos = 0;

    pdbg_targets_init(nullptr);

    trace::inf("retrieving fapi pos.");
    uint32_t attr     = std::numeric_limits<uint32_t>::max();
    pdbg_target* trgt = pdbg_target_from_path(nullptr, dimm0_path);
    pdbg_target_get_attribute(trgt, "index", 4, 1, &attr);
    trace::inf("index in DTS: %u", attr);
    EXPECT_EQ(attr, index);

    attr = std::numeric_limits<uint32_t>::max();
    pdbg_target_get_attribute(trgt, "ATTR_FAPI_POS", 4, 1, &attr);
    trace::inf("fapi pos in DTS: %u", attr);
    EXPECT_EQ(attr, fapi_pos);
}

TEST(PDBG, PdbgDtsTest3)
{
    const uint32_t chipId  = 0; // ID for proc0.
    const uint32_t fapiPos = 0; // FAPI Position for proc0.

    pdbg_targets_init(nullptr);

    // Iterate each processor.
    pdbg_target* procTrgt;
    pdbg_for_each_class_target("/proc0", procTrgt)
    {
        // Active processors only.
        if (PDBG_TARGET_ENABLED !=
            pdbg_target_probe(util::pdbg::getPibTrgt(procTrgt)))
            continue;

        // Process the PROC target.
        uint32_t attr = std::numeric_limits<uint32_t>::max();
        pdbg_target_get_attribute(procTrgt, "ATTR_CHIP_ID", 4, 1, &attr);
        trace::inf("Chip ID: %u", attr);
        EXPECT_EQ(attr, chipId);

        attr = std::numeric_limits<uint32_t>::max();
        pdbg_target_get_attribute(procTrgt, "ATTR_FAPI_POS", 4, 1, &attr);
        trace::inf("ATTR_FAPI_POS: %u", attr);
        EXPECT_EQ(attr, fapiPos);
    }
}

TEST(PDBG, PdbgDtsTest4)
{
    const uint32_t index    = 1;
    const uint32_t fapi_pos = 1;
    const char* perv1_path  = "/proc0/pib/perv1";

    pdbg_targets_init(nullptr);

    // Iterate each processor.
    pdbg_target* trgt;
    uint32_t attr;

    pdbg_for_each_class_target(perv1_path, trgt)
    {
        attr = std::numeric_limits<uint32_t>::max();
        pdbg_target_get_attribute(trgt, "index", 4, 1, &attr);
        trace::inf("index in DTS: %u", attr);
        EXPECT_EQ(attr, index);

        attr = std::numeric_limits<uint32_t>::max();
        pdbg_target_get_attribute(trgt, "ATTR_FAPI_POS", 4, 1, &attr);
        trace::inf("fapi pos in DTS: %u", attr);
        EXPECT_EQ(attr, fapi_pos);
    }
}

TEST(util_pdbg, getParentChip)
{
    using namespace util::pdbg;
    pdbg_targets_init(nullptr);

    auto procChip = getTrgt("/proc0");
    auto omiUnit  = getTrgt("/proc0/pib/perv13/mc1/mi0/mcc0/omi1");

    EXPECT_EQ(procChip, getParentChip(procChip)); // get self
    EXPECT_EQ(procChip, getParentChip(omiUnit));  // get unit

    auto ocmbChip = getTrgt("/proc0/pib/perv13/mc1/mi0/mcc0/omi1/ocmb0");
    auto memPortUnit =
        getTrgt("/proc0/pib/perv13/mc1/mi0/mcc0/omi1/ocmb0/mem_port0");

    EXPECT_EQ(ocmbChip, getParentChip(ocmbChip));    // get self
    EXPECT_EQ(ocmbChip, getParentChip(memPortUnit)); // get unit
}

TEST(util_pdbg, getChipUnit)
{
    using namespace util::pdbg;
    pdbg_targets_init(nullptr);

    auto procChip   = getTrgt("/proc0");
    auto omiUnit    = getTrgt("/proc0/pib/perv13/mc1/mi0/mcc0/omi1");
    auto omiUnitPos = 5;

    // Get the unit and verify.
    EXPECT_EQ(omiUnit, getChipUnit(procChip, TYPE_OMI, omiUnitPos));

    // Expect an exception when passing a unit instead of a chip.
    EXPECT_THROW(getChipUnit(omiUnit, TYPE_OMI, omiUnitPos), std::logic_error);

    // Expect an exception when passing a chip type.
    EXPECT_THROW(getChipUnit(procChip, TYPE_PROC, omiUnitPos),
                 std::out_of_range);

    // Expect an exception when passing a unit type not on the target chip.
    EXPECT_THROW(getChipUnit(procChip, TYPE_MEM_PORT, omiUnitPos),
                 std::out_of_range);

    // Expect a nullptr if the target is not found.
    EXPECT_EQ(nullptr, getChipUnit(procChip, TYPE_OMI, 100));

    auto ocmbChip = getTrgt("/proc0/pib/perv13/mc1/mi0/mcc0/omi1/ocmb0");
    auto memPortUnit =
        getTrgt("/proc0/pib/perv13/mc1/mi0/mcc0/omi1/ocmb0/mem_port0");
    auto memPortUnitPos = 0;

    // Get the unit and verify.
    EXPECT_EQ(memPortUnit,
              getChipUnit(ocmbChip, TYPE_MEM_PORT, memPortUnitPos));
}

TEST(util_pdbg, getScom)
{
    using namespace util::pdbg;
    pdbg_targets_init(nullptr);

    auto procChip = getTrgt("/proc0");
    auto ocmbChip = getTrgt("/proc0/pib/perv13/mc1/mi0/mcc0/omi1/ocmb0");
    auto omiUnit  = getTrgt("/proc0/pib/perv13/mc1/mi0/mcc0/omi1");

    sim::ScomAccess& scom = sim::ScomAccess::getSingleton();
    scom.flush();
    scom.add(procChip, 0x11111111, 0x0011223344556677);
    scom.error(ocmbChip, 0x22222222);

    int rc       = 0;
    uint64_t val = 0;

    // Test good path.
    rc = getScom(procChip, 0x11111111, val);
    EXPECT_EQ(0, rc);
    EXPECT_EQ(0x0011223344556677, val);

    // Test address that has not been added to ScomAccess.
    rc = getScom(procChip, 0x33333333, val);
    EXPECT_EQ(0, rc);
    EXPECT_EQ(0, val);

    // Test SCOM error.
    rc = getScom(ocmbChip, 0x22222222, val);
    EXPECT_EQ(1, rc);

    // Test non-chip target.
    EXPECT_DEATH({ getScom(omiUnit, 0x11111111, val); }, "");
}

TEST(util_pdbg, getCfam)
{
    using namespace util::pdbg;
    pdbg_targets_init(nullptr);

    auto procChip = getTrgt("/proc0");
    auto omiUnit  = getTrgt("/proc0/pib/perv13/mc1/mi0/mcc0/omi1");

    sim::CfamAccess& cfam = sim::CfamAccess::getSingleton();
    cfam.flush();
    cfam.add(procChip, 0x11111111, 0x00112233);
    cfam.error(procChip, 0x22222222);

    int rc       = 0;
    uint32_t val = 0;

    // Test good path.
    rc = getCfam(procChip, 0x11111111, val);
    EXPECT_EQ(0, rc);
    EXPECT_EQ(0x00112233, val);

    // Test address that has not been added to CfamAccess.
    rc = getCfam(procChip, 0x33333333, val);
    EXPECT_EQ(0, rc);
    EXPECT_EQ(0, val);

    // Test CFAM error.
    rc = getCfam(procChip, 0x22222222, val);
    EXPECT_EQ(1, rc);

    // Test non-chip target.
    EXPECT_DEATH({ getCfam(omiUnit, 0x11111111, val); }, "");
}
