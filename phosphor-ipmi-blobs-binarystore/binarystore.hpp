#pragma once

#include <cstdint>
#include <string>
#include <vector>

using std::size_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;

namespace binstore
{

/**
 * @class BinaryStoreInterface is an abstraction for a storage location.
 *        Each instance would be uniquely identified by a baseId string.
 */
class BinaryStoreInterface
{
  public:
    virtual ~BinaryStoreInterface() = default;

    virtual std::string getBaseBlobId() const = 0;
    virtual std::vector<std::string> getBlobIds() const = 0;
    virtual bool canHandleBlob(const std::string& blobId) const = 0;
    virtual bool openOrCreateBlob(const std::string& blobId,
                                  uint16_t flags) = 0;
    virtual bool deleteBlob(const std::string& blobId) = 0;
    virtual std::vector<uint8_t> read(uint32_t offset,
                                      uint32_t requestedSize) = 0;
    virtual bool write(uint32_t offset, const std::vector<uint8_t>& data) = 0;
    virtual bool commit() = 0;
    virtual bool close() = 0;
    virtual bool stat() = 0;
};

} // namespace binstore
