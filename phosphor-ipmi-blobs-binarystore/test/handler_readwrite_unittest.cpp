#include "handler_unittest.hpp"

using ::testing::_;
using ::testing::Return;
using ::testing::StartsWith;

using namespace std::string_literals;
using namespace binstore;

namespace blobs
{

class BinaryStoreBlobHandlerReadWriteTest : public BinaryStoreBlobHandlerTest
{
  protected:
    static inline std::string rwTestBaseId = "/test/"s;
    static inline std::string rwTestBlobId = "/test/blob0"s;
    static inline std::vector<uint8_t> rwTestData = {0, 1, 2, 3, 4,
                                                     5, 6, 7, 8, 9};
    static inline uint16_t rwTestSessionId = 0;
    static inline uint32_t rwTestOffset = 0;
};

TEST_F(BinaryStoreBlobHandlerReadWriteTest, ReadWriteReturnsWhatStoreReturns)
{
    auto testBaseId = "/test/"s;
    auto testBlobId = "/test/blob0"s;
    uint16_t flags = OpenFlags::read;
    const std::vector<uint8_t> emptyData;
    auto bstore = std::make_unique<MockBinaryStore>();

    EXPECT_CALL(*bstore, getBaseBlobId()).WillRepeatedly(Return(testBaseId));
    EXPECT_CALL(*bstore, canHandleBlob(StartsWith(testBaseId)))
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*bstore, openOrCreateBlob(_, flags)).WillOnce(Return(true));
    EXPECT_CALL(*bstore, read(rwTestOffset, _))
        .WillOnce(Return(emptyData))
        .WillOnce(Return(rwTestData));

    EXPECT_CALL(*bstore, write(rwTestOffset, emptyData))
        .WillOnce(Return(false));
    EXPECT_CALL(*bstore, write(rwTestOffset, rwTestData))
        .WillOnce(Return(true));

    handler.addNewBinaryStore(std::move(bstore));

    EXPECT_TRUE(handler.open(rwTestSessionId, flags, testBlobId));
    EXPECT_EQ(emptyData, handler.read(rwTestSessionId, rwTestOffset, 1));
    EXPECT_EQ(rwTestData, handler.read(rwTestSessionId, rwTestOffset, 1));
    EXPECT_FALSE(handler.write(rwTestSessionId, rwTestOffset, emptyData));
    EXPECT_TRUE(handler.write(rwTestSessionId, rwTestOffset, rwTestData));
}

} // namespace blobs
