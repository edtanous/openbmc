#include <stdio.h>

#include <analyzer/plugins/plugin.hpp>
#include <analyzer/util.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include "gtest/gtest.h"

//------------------------------------------------------------------------------

using namespace analyzer;

extern bool g_lpcTimeout;

// Test #1: - no LPC timeout
TEST(LpcTimeout, TestSet1)
{
    pdbg_targets_init(nullptr);

    g_lpcTimeout = false; // no timeout

    libhei::Chip chip{util::pdbg::getTrgt("/proc0"), P10_20};

    auto plugin = PluginMap::getSingleton().get(chip.getType(), "lpc_timeout");

    ServiceData sd{libhei::Signature{}, AnalysisType::SYSTEM_CHECKSTOP,
                   libhei::IsolationData{}};

    plugin(0, chip, sd);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Priority": "H",
        "Procedure": "next_level_support"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Procedure Callout",
        "Priority": "high",
        "Procedure": "next_level_support"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}

// Test #2: - LPC timeout
TEST(LpcTimeout, TestSet2)
{
    pdbg_targets_init(nullptr);

    g_lpcTimeout = true; // force timeout

    libhei::Chip chip{util::pdbg::getTrgt("/proc0"), P10_20};

    auto plugin = PluginMap::getSingleton().get(chip.getType(), "lpc_timeout");

    ServiceData sd{libhei::Signature{}, AnalysisType::SYSTEM_CHECKSTOP,
                   libhei::IsolationData{}};

    plugin(0, chip, sd);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "/bmc0",
        "Priority": "M"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "P0",
        "Priority": "M"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "/proc0",
        "Priority": "M"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Part Callout",
        "Part Type": "PNOR",
        "Priority": "medium"
    },
    {
        "Callout Type": "Clock Callout",
        "Clock Type": "OSC_REF_CLOCK_0",
        "Priority": "medium"
    },
    {
        "Callout Type": "Hardware Callout",
        "Guard": false,
        "Priority": "medium",
        "Target": "/proc0"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}
