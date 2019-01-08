#include "binarystore.hpp"

namespace binstore
{

std::unique_ptr<BinaryStoreInterface>
    BinaryStore::createFromConfig(const std::string& baseBlobId,
                                  const std::string& sysfilePath,
                                  uint32_t offset, uint32_t maxSize)
{
    // TODO: implement sysFile parsing
    return std::make_unique<BinaryStore>(baseBlobId, 0, offset, maxSize);
}

std::string BinaryStore::getBaseBlobId() const
{
    return baseBlobId_;
}

std::vector<std::string> BinaryStore::getBlobIds() const
{
    std::vector<std::string> result;
    result.push_back(baseBlobId_);

    for (const auto& blob : blob_.blobs())
    {
        result.push_back(blob.blob_id());
    }

    return result;
}

bool BinaryStore::openOrCreateBlob(const std::string& blobId, uint16_t flags)
{
    return false;
}

bool BinaryStore::deleteBlob(const std::string& blobId)
{
    return false;
}

std::vector<uint8_t> BinaryStore::read(uint32_t offset, uint32_t requestedSize)
{
    std::vector<std::uint8_t> result;
    return result;
}

bool BinaryStore::write(uint32_t offset, const std::vector<uint8_t>& data)
{
    return false;
}

bool BinaryStore::commit()
{
    return false;
}

bool BinaryStore::close()
{
    return false;
}

bool BinaryStore::stat()
{
    return false;
}

} // namespace binstore
