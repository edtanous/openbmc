#include <stdio.h>

#include <analyzer/plugins/plugin.hpp>
#include <hei_util.hpp>
#include <test/sim-hw-access.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include "gtest/gtest.h"

using namespace analyzer;

static const auto nodeId =
    static_cast<libhei::NodeId_t>(libhei::hash<libhei::NodeId_t>("TOD_ERROR"));

TEST(TodStepCheckFault, MdmtFault)
{
    pdbg_targets_init(nullptr);

    auto proc0 = util::pdbg::getTrgt("/proc0");
    auto proc1 = util::pdbg::getTrgt("/proc1");

    libhei::Chip chip0{proc0, P10_20};
    libhei::Chip chip1{proc1, P10_20};

    sim::ScomAccess& scom = sim::ScomAccess::getSingleton();
    scom.flush();

    // TOD_ERROR[14]    = 0b1   step check on primary config master select 0
    scom.add(proc0, 0x00040030, 0x0002000000000000); // TOD_ERROR

    // TOD_PSS_MSS_STATUS[0:2] = 0b000  primary config is active
    // TOD_PSS_MSS_STATUS[12]  = 0b0    primary config master select 0
    // TOD_PSS_MSS_STATUS[13]  = 0b1    primary config master TOD
    // TOD_PSS_MSS_STATUS[14]  = 0b1    primary config master drawer
    scom.add(proc0, 0x00040008, 0x0006000000000000);

    // TOD_ERROR[17]    = 0b1   internal step check
    // TOD_ERROR[21]    = 0b1   step check on primary config slave select 1
    scom.add(proc1, 0x00040030, 0x0000440000000000); // TOD_ERROR

    // TOD_PSS_MSS_STATUS[0:2] = 0b000  primary config is active
    // TOD_PSS_MSS_STATUS[15]  = 0b1    primary config slave path select 1
    scom.add(proc1, 0x00040008, 0x0001000000000000);

    // TOD_PRI_PORT_1_CTRL[0:2] = 0b001  IOHS 1
    scom.add(proc1, 0x00040002, 0x2000000000000000);

    // TOD_ERROR(0)[14] step check error on master select 0
    libhei::Signature sig0{chip0, nodeId, 0, 14, libhei::ATTN_TYPE_CHECKSTOP};

    // TOD_ERROR(0)[17] internal step check error
    libhei::Signature sig1{chip1, nodeId, 0, 17, libhei::ATTN_TYPE_CHECKSTOP};

    // TOD_ERROR(0)[21] step check error on slave select 1
    libhei::Signature sig2{chip1, nodeId, 0, 21, libhei::ATTN_TYPE_CHECKSTOP};

    libhei::IsolationData isoData{};
    isoData.addSignature(sig0);
    isoData.addSignature(sig1);
    isoData.addSignature(sig2);
    ServiceData sd{sig1, AnalysisType::SYSTEM_CHECKSTOP, isoData};

    // Call the plugin.
    auto plugin =
        PluginMap::getSingleton().get(chip1.getType(), "tod_step_check_fault");

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
        "Priority": "M"
    },
    {
        "Deconfigured": false,
        "EntityPath": [],
        "GuardType": "GARD_Unrecoverable",
        "Guarded": true,
        "LocationCode": "/proc0",
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
        "Guard": true,
        "Priority": "medium",
        "Target": "/proc0"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}
