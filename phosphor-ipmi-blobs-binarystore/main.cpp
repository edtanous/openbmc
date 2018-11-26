#include "handler.hpp"

#include <blobs-ipmid/blobs.hpp>
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * This is required by the blob manager.
 * TODO: move the declaration to blobs.hpp since all handlers need it
 */
std::unique_ptr<blobs::GenericBlobInterface> createHandler();

#ifdef __cplusplus
}
#endif

std::unique_ptr<blobs::GenericBlobInterface> createHandler()
{
    return std::make_unique<blobs::BinaryStoreBlobHandler>();
}
