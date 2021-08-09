#include <stdio.h>

#include <analyzer/resolution.hpp>

#include "gtest/gtest.h"

// Chip string
constexpr auto chip_str = "/proc0";

// Unit paths
constexpr auto proc_str = "";
constexpr auto omi_str  = "pib/perv12/mc0/mi0/mcc0/omi0";
constexpr auto core_str = "pib/perv39/eq7/fc1/core1";

// Local implementation of this function.
namespace analyzer
{

void HardwareCalloutResolution::resolve(ServiceData& io_sd) const
{
    auto sig = io_sd.getRootCause();

    std::string fru{(const char*)sig.getChip().getChip()};
    std::string guard{fru};
    if (!iv_path.empty())
    {
        guard += "/" + iv_path;
    }

    io_sd.addCallout(std::make_shared<HardwareCallout>(fru, iv_priority));

    io_sd.addGuard(std::make_shared<Guard>(guard, iv_guard));
}

} // namespace analyzer

using namespace analyzer;

TEST(Resolution, TestSet1)
{
    // Create a few resolutions
    auto c1 = std::make_shared<HardwareCalloutResolution>(
        proc_str, Callout::Priority::HIGH, Guard::NONE);

    auto c2 = std::make_shared<HardwareCalloutResolution>(
        omi_str, Callout::Priority::MED_A, Guard::FATAL);

    auto c3 = std::make_shared<HardwareCalloutResolution>(
        core_str, Callout::Priority::MED, Guard::NON_FATAL);

    auto c4 = std::make_shared<ProcedureCalloutResolution>(
        ProcedureCallout::NEXTLVL, Callout::Priority::LOW);

    // l1 = (c1, c2)
    auto l1 = std::make_shared<ResolutionList>();
    l1->push(c1);
    l1->push(c2);

    // l2 = (c4, c3, c1, c2)
    auto l2 = std::make_shared<ResolutionList>();
    l2->push(c4);
    l2->push(c3);
    l2->push(l1);

    // Get some ServiceData objects
    libhei::Chip chip{chip_str, 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHECKSTOP};
    ServiceData sd1{sig, true};
    ServiceData sd2{sig, false};

    // Resolve
    l1->resolve(sd1);
    l2->resolve(sd2);

    // Start verifying
    nlohmann::json j{};
    std::string s{};

    sd1.getCalloutList(j);
    s = R"([
    {
        "LocationCode": "/proc0",
        "Priority": "H"
    },
    {
        "LocationCode": "/proc0",
        "Priority": "A"
    }
])";
    ASSERT_EQ(s, j.dump(4));

    sd1.getGuardList(j);
    s = R"([
    {
        "Path": "/proc0",
        "Type": "NONE"
    },
    {
        "Path": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0",
        "Type": "FATAL"
    }
])";
    ASSERT_EQ(s, j.dump(4));

    sd2.getCalloutList(j);
    s = R"([
    {
        "Priority": "L",
        "Procedure": "NEXTLVL"
    },
    {
        "LocationCode": "/proc0",
        "Priority": "M"
    },
    {
        "LocationCode": "/proc0",
        "Priority": "H"
    },
    {
        "LocationCode": "/proc0",
        "Priority": "A"
    }
])";
    ASSERT_EQ(s, j.dump(4));

    sd2.getGuardList(j);
    s = R"([
    {
        "Path": "/proc0/pib/perv39/eq7/fc1/core1",
        "Type": "NON_FATAL"
    },
    {
        "Path": "/proc0",
        "Type": "NONE"
    },
    {
        "Path": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0",
        "Type": "FATAL"
    }
])";
    ASSERT_EQ(s, j.dump(4));
}
