#include <stdio.h>

#include <analyzer/plugins/plugin.hpp>
#include <hei_util.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include "gtest/gtest.h"

using namespace analyzer;

static const auto nodeId =
    static_cast<libhei::NodeId_t>(libhei::hash<libhei::NodeId_t>("TOD_ERROR"));

TEST(TodStepCheckFault, TestSet1)
{
    pdbg_targets_init(nullptr);

    libhei::Chip chip1{util::pdbg::getTrgt("/proc1"), P10_20};

    // TOD_ERROR(0)[16]
    libhei::Signature sig{chip1, nodeId, 0, 16, libhei::ATTN_TYPE_CHECKSTOP};

    auto plugin =
        PluginMap::getSingleton().get(chip1.getType(), "tod_step_check_fault");

    libhei::IsolationData isoData{};
    isoData.addSignature(sig);
    ServiceData sd{sig, AnalysisType::SYSTEM_CHECKSTOP, isoData};

    // Call the plugin.
    plugin(1, chip1, sd);

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
        "Clock Type": "TOD_CLOCK",
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
