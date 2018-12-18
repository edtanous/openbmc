#pragma once

#include "binarystore.hpp"

#include <gmock/gmock.h>

namespace binstore
{

class MockBinaryStore : public BinaryStoreInterface
{
  public:
    MOCK_CONST_METHOD0(getBaseBlobId, std::string());
    MOCK_CONST_METHOD1(canHandleBlob, bool(const std::string&));
    MOCK_CONST_METHOD0(getBlobIds, std::vector<std::string>());
    MOCK_METHOD1(openOrCreateBlob, bool(const std::string&));
    MOCK_METHOD2(read, std::vector<uint8_t>(uint32_t, uint32_t));
    MOCK_METHOD2(write, bool(uint32_t, const std::vector<uint8_t>&));
    MOCK_METHOD0(commit, bool());
    MOCK_METHOD0(close, bool());
    MOCK_METHOD0(stat, bool());
};

} // namespace binstore
