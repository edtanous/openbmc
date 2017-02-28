/**
 * Copyright © 2017 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>
#include <experimental/filesystem>
#include <fstream>
#include <stdlib.h>
#include "targeting.hpp"

using namespace openpower::targeting;
namespace fs = std::experimental::filesystem;

constexpr auto masterDir = "/tmp";

class TargetingTest : public ::testing::Test
{
    protected:

        virtual void SetUp()
        {
            char dir[50];
            strcpy(dir, masterDir);
            strcat(dir, "/targetingXXXXXX");

            auto path = mkdtemp(dir);
            assert(path != nullptr);

            _directory = path;
        }

        virtual void TearDown()
        {
            fs::remove_all(_directory);
        }

        std::string _directory;
};


TEST_F(TargetingTest, CreateTargets)
{

    //Test that we always create the first Target
    {
        Targeting targets{masterDir, _directory};
        ASSERT_EQ(targets.size(), 1);

        auto t = targets.begin();
        ASSERT_EQ((*t)->getPos(), 0);

        ASSERT_EQ((*t)->getPath(), masterDir);
    }


    //Test that we can create multiple Targets
    {
        //make some fake slave entries
        std::ofstream(_directory + "/slave@01:00");
        std::ofstream(_directory + "/slave@02:00");
        std::ofstream(_directory + "/slave@03:00");
        std::ofstream(_directory + "/slave@04:00");

        Targeting targets{masterDir, _directory};

        ASSERT_EQ(targets.size(), 5);

        int i = 0;

        for (const auto& t : targets)
        {
            std::ostringstream path;

            ASSERT_EQ(t->getPos(), i);

            if (0 == i)
            {
                path << masterDir;
            }
            else
            {
                path << _directory << "/slave@0" << i << ":00/raw";
            }

            ASSERT_EQ(t->getPath(), path.str());
            i++;
        }
    }
}
