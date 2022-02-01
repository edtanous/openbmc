#include <stdio.h>

#include <analyzer/plugins/plugin.hpp>
#include <analyzer/ras-data/ras-data-parser.hpp>
#include <hei_util.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include "gtest/gtest.h"

using namespace analyzer;

static const auto nodeId =
    static_cast<libhei::NodeId_t>(libhei::hash<libhei::NodeId_t>("PLL_UNLOCK"));

// Sub-test #1 - single PLL unlock attention on proc 1, clock 1
TEST(PllUnlock, TestSet1)
{
    pdbg_targets_init(nullptr);

    libhei::Chip chip1{util::pdbg::getTrgt("/proc1"), P10_20};

    libhei::Signature sig11{chip1, nodeId, 0, 1, libhei::ATTN_TYPE_CHECKSTOP};

    libhei::IsolationData isoData{};
    isoData.addSignature(sig11);
    ServiceData sd{sig11, AnalysisType::SYSTEM_CHECKSTOP, isoData};

    RasDataParser rasData{};
    rasData.getResolution(sig11)->resolve(sd);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "P0",
        "Priority": "M"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "/proc1",
        "Priority": "M"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Clock Callout",
        "Clock Type": "OSC_REF_CLOCK_1",
        "Priority": "medium"
    },
    {
        "Callout Type": "Hardware Callout",
        "Guard": false,
        "Priority": "medium",
        "Target": "/proc1"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}

// Sub-test #2 - PLL unlock attention on multiple procs and clocks. Isolating
//               only to proc 1 clock 0 PLL unlock attentions.
TEST(PllUnlock, TestSet2)
{
    pdbg_targets_init(nullptr);

    libhei::Chip chip0{util::pdbg::getTrgt("/proc0"), P10_20};
    libhei::Chip chip1{util::pdbg::getTrgt("/proc1"), P10_20};

    // PLL unlock signatures for each clock per processor.
    libhei::Signature sig00{chip0, nodeId, 0, 0, libhei::ATTN_TYPE_CHECKSTOP};
    libhei::Signature sig01{chip0, nodeId, 0, 1, libhei::ATTN_TYPE_CHECKSTOP};
    libhei::Signature sig10{chip1, nodeId, 0, 0, libhei::ATTN_TYPE_CHECKSTOP};
    libhei::Signature sig11{chip1, nodeId, 0, 1, libhei::ATTN_TYPE_CHECKSTOP};

    // Plugins for each processor.
    auto plugin = PluginMap::getSingleton().get(chip1.getType(), "pll_unlock");

    libhei::IsolationData isoData{};
    isoData.addSignature(sig00);
    isoData.addSignature(sig01);
    isoData.addSignature(sig10);
    isoData.addSignature(sig11);
    ServiceData sd{sig10, AnalysisType::SYSTEM_CHECKSTOP, isoData};

    // Call the PLL unlock plugin.
    plugin(0, chip1, sd);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "P0",
        "Priority": "H"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "/proc0",
        "Priority": "M"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "/proc1",
        "Priority": "M"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Clock Callout",
        "Clock Type": "OSC_REF_CLOCK_0",
        "Priority": "high"
    },
    {
        "Callout Type": "Hardware Callout",
        "Guard": false,
        "Priority": "medium",
        "Target": "/proc0"
    },
    {
        "Callout Type": "Hardware Callout",
        "Guard": false,
        "Priority": "medium",
        "Target": "/proc1"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}
