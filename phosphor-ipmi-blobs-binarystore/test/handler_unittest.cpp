#include "handler_unittest.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::StrNe;
using namespace std::string_literals;
using namespace binstore;

namespace blobs
{

class BinaryStoreBlobHandlerBasicTest : public BinaryStoreBlobHandlerTest
{
  protected:
    static inline std::string basicTestBaseId = "/test/"s;
    static inline std::string basicTestBlobId = "/test/blob0"s;
    static inline std::string basicTestInvalidBlobId = "/invalid/blob0"s;
};

TEST_F(BinaryStoreBlobHandlerBasicTest, CanHandleBlobZeroStoreFail)
{
    // Cannot handle since there is no store. Shouldn't crash.
    EXPECT_FALSE(handler.canHandleBlob(basicTestInvalidBlobId));
}

TEST_F(BinaryStoreBlobHandlerBasicTest, CanHandleBlobChecksNameInvalid)
{
    auto bstore = std::make_unique<MockBinaryStore>();

    handler.addNewBinaryStore(std::move(bstore));

    // Verify canHandleBlob checks and returns false on an invalid name.
    EXPECT_FALSE(handler.canHandleBlob(basicTestInvalidBlobId));
}

TEST_F(BinaryStoreBlobHandlerBasicTest, CanHandleBlobCanOpenValidBlob)
{
    auto bstore = std::make_unique<MockBinaryStore>();

    EXPECT_CALL(*bstore, getBaseBlobId())
        .WillRepeatedly(Return(basicTestBaseId));
    EXPECT_CALL(*bstore, canHandleBlob(StrNe(basicTestBlobId)))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(*bstore, canHandleBlob(StrEq(basicTestBlobId)))
        .WillRepeatedly(Return(true));
    handler.addNewBinaryStore(std::move(bstore));

    // Verify canHandleBlob return true for a blob id that it can handle
    EXPECT_FALSE(handler.canHandleBlob(basicTestInvalidBlobId));
    EXPECT_TRUE(handler.canHandleBlob(basicTestBlobId));
}

TEST_F(BinaryStoreBlobHandlerBasicTest, CanHandleBlobCanOpenValidBlobMultiple)
{
    auto bstore = std::make_unique<MockBinaryStore>();
    auto bstore1 = std::make_unique<MockBinaryStore>();
    const std::string anotherBaseId = "/another/"s;
    const std::string anotherBlobId = "/another/blob/id"s;

    EXPECT_CALL(*bstore, getBaseBlobId())
        .WillRepeatedly(Return(basicTestBaseId));
    EXPECT_CALL(*bstore, canHandleBlob(StrNe(basicTestBlobId)))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(*bstore, canHandleBlob(StrEq(basicTestBlobId)))
        .WillRepeatedly(Return(true));
    handler.addNewBinaryStore(std::move(bstore));

    EXPECT_CALL(*bstore1, getBaseBlobId())
        .WillRepeatedly(Return(anotherBaseId));
    EXPECT_CALL(*bstore1, canHandleBlob(StrNe(anotherBlobId)))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(*bstore1, canHandleBlob(StrEq(anotherBlobId)))
        .WillRepeatedly(Return(true));
    handler.addNewBinaryStore(std::move(bstore1));

    // Verify canHandleBlob return true for a blob id that it can handle
    EXPECT_FALSE(handler.canHandleBlob(basicTestInvalidBlobId));
    EXPECT_TRUE(handler.canHandleBlob(basicTestBlobId));
    EXPECT_TRUE(handler.canHandleBlob(anotherBlobId));
}

TEST_F(BinaryStoreBlobHandlerBasicTest, GetBlobIdEqualsConcatenationsOfIds)
{
    std::string baseId0 = "/test/"s;
    std::string baseId1 = "/test1/"s;
    std::vector<std::string> idList0 = {"/test/"s, "/test/0"s};
    std::vector<std::string> idList1 = {"/test1/"s, "/test1/2"s};
    auto expectedIdList = idList0;
    expectedIdList.insert(expectedIdList.end(), idList1.begin(), idList1.end());

    auto bstore0 = std::make_unique<MockBinaryStore>();
    EXPECT_CALL(*bstore0, getBaseBlobId()).WillOnce(Return(baseId0));
    EXPECT_CALL(*bstore0, getBlobIds()).WillOnce(Return(idList0));
    handler.addNewBinaryStore(std::move(bstore0));

    auto bstore1 = std::make_unique<MockBinaryStore>();
    EXPECT_CALL(*bstore1, getBaseBlobId()).WillOnce(Return(baseId1));
    EXPECT_CALL(*bstore1, getBlobIds()).WillOnce(Return(idList1));
    handler.addNewBinaryStore(std::move(bstore1));

    // Verify canHandleBlob return true for a blob id that it can handle
    EXPECT_EQ(expectedIdList, handler.getBlobIds());
}

TEST_F(BinaryStoreBlobHandlerBasicTest, DeleteReturnsWhatStoreReturns)
{
    auto bstore = std::make_unique<MockBinaryStore>();

    EXPECT_CALL(*bstore, getBaseBlobId())
        .WillRepeatedly(Return(basicTestBaseId));
    EXPECT_CALL(*bstore, canHandleBlob(StrNe(basicTestBlobId)))
        .WillRepeatedly(Return(false));
    EXPECT_CALL(*bstore, canHandleBlob(StrEq(basicTestBlobId)))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*bstore, deleteBlob(StrEq(basicTestBlobId)))
        .WillOnce(Return(false))
        .WillOnce(Return(true));
    handler.addNewBinaryStore(std::move(bstore));

    // Verify canHandleBlob return true for a blob id that it can handle
    EXPECT_FALSE(handler.canHandleBlob(basicTestInvalidBlobId));
    EXPECT_TRUE(handler.canHandleBlob(basicTestBlobId));
    EXPECT_FALSE(handler.deleteBlob(basicTestInvalidBlobId));
    EXPECT_FALSE(handler.deleteBlob(basicTestBlobId));
    EXPECT_TRUE(handler.deleteBlob(basicTestBlobId));
}

} // namespace blobs
