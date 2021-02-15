#include <stdio.h>

#include <analyzer/service_data.hpp>

#include "gtest/gtest.h"

using namespace analyzer;

TEST(SericeData, TestSet1)
{
    HardwareCallout c1{"Test location 1", Callout::Priority::HIGH};
    HardwareCallout c2{"Test location 2", Callout::Priority::MED_A};
    ProcedureCallout c3{ProcedureCallout::NEXTLVL, Callout::Priority::LOW};

    nlohmann::json j{};

    c1.getJson(j);
    c2.getJson(j);
    c3.getJson(j);

    // Create a RAW string containing what we should expect in the JSON output.
    std::string s = R"([
    {
        "LocationCode": "Test location 1",
        "Priority": "H"
    },
    {
        "LocationCode": "Test location 2",
        "Priority": "A"
    },
    {
        "Priority": "L",
        "Procedure": "NEXTLVL"
    }
])";

    ASSERT_EQ(s, j.dump(4));
}
