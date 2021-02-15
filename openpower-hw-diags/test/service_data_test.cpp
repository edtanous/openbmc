#include <stdio.h>

#include <analyzer/service_data.hpp>

#include "gtest/gtest.h"

using namespace analyzer;

TEST(SericeData, TestSet1)
{
    ServiceData sd{};

    sd.addCallout(std::make_shared<HardwareCallout>("Test location 1",
                                                    Callout::Priority::HIGH));
    sd.addCallout(std::make_shared<HardwareCallout>("Test location 2",
                                                    Callout::Priority::MED_A));
    sd.addCallout(std::make_shared<ProcedureCallout>(ProcedureCallout::NEXTLVL,
                                                     Callout::Priority::LOW));

    nlohmann::json j{};
    sd.getCalloutList(j);

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
