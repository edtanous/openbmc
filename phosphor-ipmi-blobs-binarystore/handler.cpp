#include "handler.hpp"

namespace blobs
{

bool BinaryStoreBlobHandler::canHandleBlob(const std::string& path)
{
    // TODO: implement
    return false;
}

std::vector<std::string> BinaryStoreBlobHandler::getBlobIds()
{
    // TODO: implement
    std::vector<std::string> result;
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
    // TODO: implement
    return false;
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
