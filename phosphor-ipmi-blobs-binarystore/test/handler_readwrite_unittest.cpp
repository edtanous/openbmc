#include "handler_unittest.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

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
    uint16_t flags = OpenFlags::read;
    const std::vector<uint8_t> emptyData;
    auto store = defaultMockStore(rwTestBaseId);

    EXPECT_CALL(*store, openOrCreateBlob(_, flags)).WillOnce(Return(true));
    EXPECT_CALL(*store, read(rwTestOffset, _))
        .WillOnce(Return(emptyData))
        .WillOnce(Return(rwTestData));

    EXPECT_CALL(*store, write(rwTestOffset, emptyData)).WillOnce(Return(false));
    EXPECT_CALL(*store, write(rwTestOffset, rwTestData)).WillOnce(Return(true));

    handler.addNewBinaryStore(std::move(store));

    EXPECT_TRUE(handler.open(rwTestSessionId, flags, rwTestBlobId));
    EXPECT_EQ(emptyData, handler.read(rwTestSessionId, rwTestOffset, 1));
    EXPECT_EQ(rwTestData, handler.read(rwTestSessionId, rwTestOffset, 1));
    EXPECT_FALSE(handler.write(rwTestSessionId, rwTestOffset, emptyData));
    EXPECT_TRUE(handler.write(rwTestSessionId, rwTestOffset, rwTestData));
}

} // namespace blobs
