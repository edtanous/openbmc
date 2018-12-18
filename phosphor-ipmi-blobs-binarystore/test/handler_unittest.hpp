#pragma once

#include "binarystore_mock.hpp"
#include "handler.hpp"

#include <gtest/gtest.h>

namespace blobs
{

class BinaryStoreBlobHandlerTest : public ::testing::Test
{
  protected:
    BinaryStoreBlobHandlerTest() = default;
    BinaryStoreBlobHandler handler;
};

} // namespace blobs
