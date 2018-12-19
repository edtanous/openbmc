#include "handler_unittest.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::StartsWith;

using namespace std::string_literals;
using namespace binstore;

namespace blobs
{

TEST_F(BinaryStoreBlobHandlerOpenTest, FailWhenCannotHandleId)
{
    uint16_t flags = OpenFlags::read, sessionId = 0;
    EXPECT_FALSE(handler.open(sessionId, flags, "/invalid/blob"s));
}

TEST_F(BinaryStoreBlobHandlerOpenTest, FailWhenStoreOpenReturnsFailure)
{
    auto testBaseId = "/test/"s;
    auto testBlobId = "/test/blob0"s;
    uint16_t flags = OpenFlags::read, sessionId = 0;
    auto bstore = std::make_unique<MockBinaryStore>();

    EXPECT_CALL(*bstore, getBaseBlobId()).WillRepeatedly(Return(testBaseId));
    EXPECT_CALL(*bstore, canHandleBlob(StartsWith(testBaseId)))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*bstore, openOrCreateBlob(_, flags)).WillOnce(Return(false));

    handler.addNewBinaryStore(std::move(bstore));

    EXPECT_FALSE(handler.open(sessionId, flags, testBlobId));
}

TEST_F(BinaryStoreBlobHandlerOpenTest, SucceedWhenStoreOpenReturnsTrue)
{
    auto testBaseId = "/test/"s;
    auto testBlobId = "/test/blob0"s;
    uint16_t flags = OpenFlags::read, sessionId = 0;
    auto bstore = std::make_unique<MockBinaryStore>();

    EXPECT_CALL(*bstore, getBaseBlobId()).WillRepeatedly(Return(testBaseId));
    EXPECT_CALL(*bstore, canHandleBlob(StartsWith(testBaseId)))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*bstore, openOrCreateBlob(_, flags)).WillOnce(Return(true));

    handler.addNewBinaryStore(std::move(bstore));

    EXPECT_TRUE(handler.open(sessionId, flags, testBlobId));
}

TEST_F(BinaryStoreBlobHandlerOpenTest, CloseFailForInvalidSession)
{
    uint16_t invalidSessionId = 1;
    EXPECT_FALSE(handler.close(invalidSessionId));
}

TEST_F(BinaryStoreBlobHandlerOpenTest, CloseFailWhenStoreCloseFails)
{
    auto testBaseId = "/test/"s;
    auto testBlobId = "/test/blob0"s;
    uint16_t flags = OpenFlags::read, sessionId = 0;
    auto bstore = std::make_unique<MockBinaryStore>();

    EXPECT_CALL(*bstore, getBaseBlobId()).WillRepeatedly(Return(testBaseId));
    EXPECT_CALL(*bstore, canHandleBlob(StartsWith(testBaseId)))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*bstore, openOrCreateBlob(_, flags)).WillOnce(Return(true));
    EXPECT_CALL(*bstore, close()).WillOnce(Return(false));

    handler.addNewBinaryStore(std::move(bstore));

    EXPECT_TRUE(handler.open(sessionId, flags, testBlobId));
    EXPECT_FALSE(handler.close(sessionId));
}

TEST_F(BinaryStoreBlobHandlerOpenTest, CloseSucceedWhenStoreCloseSucceeds)
{
    auto testBaseId = "/test/"s;
    auto testBlobId = "/test/blob0"s;
    uint16_t flags = OpenFlags::read, sessionId = 0;
    auto bstore = std::make_unique<MockBinaryStore>();

    EXPECT_CALL(*bstore, getBaseBlobId()).WillRepeatedly(Return(testBaseId));
    EXPECT_CALL(*bstore, canHandleBlob(StartsWith(testBaseId)))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*bstore, openOrCreateBlob(_, flags)).WillOnce(Return(true));
    EXPECT_CALL(*bstore, close()).WillOnce(Return(true));

    handler.addNewBinaryStore(std::move(bstore));

    EXPECT_TRUE(handler.open(sessionId, flags, testBlobId));
    EXPECT_TRUE(handler.close(sessionId));
}

TEST_F(BinaryStoreBlobHandlerOpenTest, ClosedSessionCannotBeReclosed)
{
    auto testBaseId = "/test/"s;
    auto testBlobId = "/test/blob0"s;
    uint16_t flags = OpenFlags::read, sessionId = 0;
    auto bstore = std::make_unique<MockBinaryStore>();

    EXPECT_CALL(*bstore, getBaseBlobId()).WillRepeatedly(Return(testBaseId));
    EXPECT_CALL(*bstore, canHandleBlob(StartsWith(testBaseId)))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*bstore, openOrCreateBlob(_, flags)).WillOnce(Return(true));
    EXPECT_CALL(*bstore, close()).WillRepeatedly(Return(true));

    handler.addNewBinaryStore(std::move(bstore));

    EXPECT_TRUE(handler.open(sessionId, flags, testBlobId));
    EXPECT_TRUE(handler.close(sessionId));
    EXPECT_FALSE(handler.close(sessionId));
}

} // namespace blobs
