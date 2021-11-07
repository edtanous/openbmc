#include <ipmiblob/blob_interface.hpp>

#include <gmock/gmock.h>

namespace ipmiblob
{

class BlobInterfaceMock : public BlobInterface
{
  public:
    virtual ~BlobInterfaceMock() = default;
    MOCK_METHOD(void, commit, (std::uint16_t, const std::vector<std::uint8_t>&),
                (override));
    MOCK_METHOD(void, writeMeta,
                (std::uint16_t, std::uint32_t,
                 const std::vector<std::uint8_t>&),
                (override));
    MOCK_METHOD(void, writeBytes,
                (std::uint16_t, std::uint32_t,
                 const std::vector<std::uint8_t>&),
                (override));
    MOCK_METHOD(std::vector<std::string>, getBlobList, (), (override));
    MOCK_METHOD(StatResponse, getStat, (const std::string&), (override));
    MOCK_METHOD(StatResponse, getStat, (std::uint16_t), (override));
    MOCK_METHOD(std::uint16_t, openBlob, (const std::string&, std::uint16_t),
                (override));
    MOCK_METHOD(void, closeBlob, (std::uint16_t), (override));
    MOCK_METHOD(bool, deleteBlob, (const std::string&), (override));
    MOCK_METHOD(std::vector<std::uint8_t>, readBytes,
                (std::uint16_t, std::uint32_t, std::uint32_t), (override));
};

} // namespace ipmiblob
