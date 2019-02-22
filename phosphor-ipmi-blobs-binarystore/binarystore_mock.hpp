#pragma once

#include "binarystore.hpp"

#include <gmock/gmock.h>

using ::testing::Invoke;

namespace binstore
{

class MockBinaryStore : public BinaryStoreInterface
{
  public:
    MockBinaryStore(const std::string& baseBlobId, int fd, uint32_t offset,
                    uint32_t maxSize) :
        real_store_(baseBlobId, fd, offset, maxSize)
    {
        // Implemented calls in BinaryStore will be directed to the real object.
        ON_CALL(*this, getBaseBlobId)
            .WillByDefault(Invoke(&real_store_, &BinaryStore::getBaseBlobId));
        ON_CALL(*this, getBlobIds)
            .WillByDefault(Invoke(&real_store_, &BinaryStore::getBlobIds));
        ON_CALL(*this, openOrCreateBlob)
            .WillByDefault(
                Invoke(&real_store_, &BinaryStore::openOrCreateBlob));
        ON_CALL(*this, close)
            .WillByDefault(Invoke(&real_store_, &BinaryStore::close));
        ON_CALL(*this, read)
            .WillByDefault(Invoke(&real_store_, &BinaryStore::read));
        ON_CALL(*this, write)
            .WillByDefault(Invoke(&real_store_, &BinaryStore::write));
    }
    MOCK_CONST_METHOD0(getBaseBlobId, std::string());
    MOCK_CONST_METHOD0(getBlobIds, std::vector<std::string>());
    MOCK_METHOD2(openOrCreateBlob, bool(const std::string&, uint16_t));
    MOCK_METHOD1(deleteBlob, bool(const std::string&));
    MOCK_METHOD2(read, std::vector<uint8_t>(uint32_t, uint32_t));
    MOCK_METHOD2(write, bool(uint32_t, const std::vector<uint8_t>&));
    MOCK_METHOD0(commit, bool());
    MOCK_METHOD0(close, bool());
    MOCK_METHOD0(stat, bool());

  private:
    BinaryStore real_store_;
};

} // namespace binstore
