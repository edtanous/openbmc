#include "handler_unittest.hpp"

namespace blobs
{

TEST_F(BinaryStoreBlobHandlerBasicTest, CanHandleBlobChecksNameInvalid)
{
    // Verify canHandleBlob checks and returns false on an invalid name.
    EXPECT_FALSE(bstore.canHandleBlob("asdf"));
}

} // namespace blobs
