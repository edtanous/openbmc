#include <util/ffdc_file.hpp>
#include <util/trace.hpp>

#include "gtest/gtest.h"

TEST(FFDCFile, TestSet1)
{
    std::vector<util::FFDCFile> files;
    files.emplace_back(util::FFDCFormat::JSON, 1, 1);
    files.emplace_back(util::FFDCFormat::CBOR, 2, 2);
    files.emplace_back(util::FFDCFormat::Text, 3, 3);
    files.emplace_back(util::FFDCFormat::Custom, 4, 4);

    std::vector<util::FFDCTuple> tuples;
    ASSERT_NO_THROW(util::transformFFDC(files, tuples));
}
