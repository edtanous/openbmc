#pragma once

#include "binarystore_mock.hpp"
#include "handler.hpp"

#include <memory>
#include <string>

#include <gtest/gtest.h>

using ::testing::Contains;

using namespace std::string_literals;

namespace blobs
{

class BinaryStoreBlobHandlerTest : public ::testing::Test
{
  protected:
    BinaryStoreBlobHandlerTest() = default;
    BinaryStoreBlobHandler handler;

    std::unique_ptr<binstore::MockBinaryStore>
        defaultMockStore(const std::string& baseId)
    {
        return std::make_unique<binstore::MockBinaryStore>(baseId, 0, 0, 0);
    }

    void addDefaultStore(const std::string& baseId)
    {
        handler.addNewBinaryStore(defaultMockStore(baseId));
    }
};

} // namespace blobs
