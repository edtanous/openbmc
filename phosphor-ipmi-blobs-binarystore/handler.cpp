#include "handler.hpp"

#include <algorithm>

namespace blobs
{

namespace internal
{

/* Strip the basename till the last '/' */
std::string getBaseFromId(const std::string& blobId)
{
    return blobId.substr(0, blobId.find_last_of('/') + 1);
}

} // namespace internal

void BinaryStoreBlobHandler::addNewBinaryStore(
    std::unique_ptr<binstore::BinaryStoreInterface> store)
{
    // TODO: this is a very rough measure to test the mock interface for now.
    stores_[store->getBaseBlobId()] = std::move(store);
}

bool BinaryStoreBlobHandler::canHandleBlob(const std::string& path)
{
    return std::any_of(stores_.begin(), stores_.end(),
                       [&](const auto& baseStorePair) {
                           return baseStorePair.second->canHandleBlob(path);
                       });
}

std::vector<std::string> BinaryStoreBlobHandler::getBlobIds()
{
    std::vector<std::string> result;

    for (const auto& baseStorePair : stores_)
    {
        const auto& ids = baseStorePair.second->getBlobIds();
        result.insert(result.end(), ids.begin(), ids.end());
    }

    return result;
}

bool BinaryStoreBlobHandler::deleteBlob(const std::string& path)
{
    // TODO: implement
    return false;
}

bool BinaryStoreBlobHandler::stat(const std::string& path,
                                  struct BlobMeta* meta)
{
    // TODO: implement
    return false;
}

bool BinaryStoreBlobHandler::open(uint16_t session, uint16_t flags,
                                  const std::string& path)
{
    if (!canHandleBlob(path))
    {
        return false;
    }

    auto found = sessions_.find(session);
    if (found != sessions_.end())
    {
        /* This session is already active */
        return false;
    }

    const auto& base = internal::getBaseFromId(path);

    if (stores_.find(base) == stores_.end())
    {
        return false;
    }

    if (!stores_[base]->openOrCreateBlob(path, flags))
    {
        return false;
    }

    sessions_[session] = stores_[base].get();
    return true;
}

std::vector<uint8_t> BinaryStoreBlobHandler::read(uint16_t session,
                                                  uint32_t offset,
                                                  uint32_t requestedSize)
{
    // TODO: implement
    std::vector<uint8_t> result;
    return result;
}

bool BinaryStoreBlobHandler::write(uint16_t session, uint32_t offset,
                                   const std::vector<uint8_t>& data)
{
    // TODO: implement
    return false;
}

bool BinaryStoreBlobHandler::writeMeta(uint16_t session, uint32_t offset,
                                       const std::vector<uint8_t>& data)
{
    /* Binary store handler doesn't support write meta */
    return false;
}

bool BinaryStoreBlobHandler::commit(uint16_t session,
                                    const std::vector<uint8_t>& data)
{
    // TODO: implement
    return false;
}

bool BinaryStoreBlobHandler::close(uint16_t session)
{
    // TODO: implement
    return false;
}

bool BinaryStoreBlobHandler::stat(uint16_t session, struct BlobMeta* meta)
{
    // TODO: implement
    return false;
}

bool BinaryStoreBlobHandler::expire(uint16_t session)
{
    /* Binary store handler doesn't support expire */
    return false;
}

} // namespace blobs
