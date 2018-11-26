#pragma once

#include "handler.hpp"

#include <gtest/gtest.h>

namespace blobs
{

class BinaryStoreBlobHandlerTest : public ::testing::Test
{
  protected:
    BinaryStoreBlobHandlerTest() = default;

    BinaryStoreBlobHandler bstore;
};

class BinaryStoreBlobHandlerBasicTest : public BinaryStoreBlobHandlerTest
{
};

} // namespace blobs
