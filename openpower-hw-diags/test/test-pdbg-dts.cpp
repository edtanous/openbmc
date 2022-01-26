#include <fcntl.h>

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
