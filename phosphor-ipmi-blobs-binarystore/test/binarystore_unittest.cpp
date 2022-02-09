#include "binarystore.hpp"
#include "binarystore_interface.hpp"
#include "sys_file.hpp"

#include <google/protobuf/text_format.h>

#include <iostream>
#include <ipmid/handler.hpp>
#include <iterator>
#include <memory>
#include <sstream>
#include <vector>

#include <gmock/gmock.h>

const std::string blobData = "jW7jiID}kD&gm&Azi:^]JT]'Ov4"
                             "Y.Oey]mw}yak9Wf3[S`+$!g]@[0}gikis^";
const std::string inputProto = "blob_base_id: \"/blob/my-test\""
                               "blobs: [{ "
                               "    blob_id: \"/blob/my-test/0\""
                               "    data:\"" +
                               blobData +
                               "\""
                               "}, { "
                               "    blob_id: \"/blob/my-test/1\""
                               "    data:\"" +
                               blobData +
                               "\""
                               "}, { "
                               "    blob_id: \"/blob/my-test/2\""
                               "    data:\"" +
                               blobData +
                               "\""
                               "}, {"
                               "    blob_id: \"/blob/my-test/3\""
                               "    data:\"" +
                               blobData +
                               "\""
                               "}] "
                               "max_size_bytes: 64";
const std::string smallInputProto = "blob_base_id: \"/s/test\""
                                    "blobs: [{ "
                                    "    blob_id: \"/s/test/0\""
                                    "}] "
                                    "max_size_bytes: 64";

class SysFileBuf : public binstore::SysFile
{
  public:
    explicit SysFileBuf(std::string* storage) : data_{storage}
    {
    }

    size_t readToBuf(size_t pos, size_t count, char* buf) const override
    {
        std::cout << "Read " << count << " bytes at " << pos << std::endl;
        return data_->copy(buf, count, pos);
    }

    std::string readAsStr(size_t pos, size_t count) const override
    {
        std::cout << "Read as str " << count << " bytes at " << pos
                  << std::endl;
        return data_->substr(pos, count);
    }

    std::string readRemainingAsStr(size_t pos) const override
    {
        return data_->substr(pos);
    }

    void writeStr(const std::string& data, size_t pos) override
    {
        data_->replace(pos, data.size(), data);
    }

    std::string* data_;
};

using binstore::binaryblobproto::BinaryBlobBase;
using google::protobuf::TextFormat;

using testing::ElementsAreArray;
using testing::UnorderedElementsAre;

class BinaryStoreTest : public testing::Test
{
  public:
    std::unique_ptr<SysFileBuf> createBlobStorage(const std::string& textProto)
    {
        BinaryBlobBase storeProto;
        TextFormat::ParseFromString(textProto, &storeProto);

        std::stringstream storage;
        std::string data;
        storeProto.SerializeToString(&data);

        const uint64_t dataSize = data.size();
        storage.write(reinterpret_cast<const char*>(&dataSize),
                      sizeof(dataSize));
        storage << data;

        blobDataStorage = storage.str();
        return std::make_unique<SysFileBuf>(&blobDataStorage);
    }

    std::string blobDataStorage;
};

TEST_F(BinaryStoreTest, SimpleLoad)
{
    auto testDataFile = createBlobStorage(inputProto);
    auto initialData = blobDataStorage;
    auto store = binstore::BinaryStore::createFromConfig(
        "/blob/my-test", std::move(testDataFile));
    EXPECT_THAT(store->getBlobIds(),
                UnorderedElementsAre("/blob/my-test", "/blob/my-test/0",
                                     "/blob/my-test/1", "/blob/my-test/2",
                                     "/blob/my-test/3"));
    EXPECT_EQ(initialData, blobDataStorage);
}

TEST_F(BinaryStoreTest, TestCreateFromFile)
{
    auto testDataFile = createBlobStorage(inputProto);
    auto initialData = blobDataStorage;
    auto store =
        binstore::BinaryStore::createFromFile(std::move(testDataFile), true);
    ASSERT_TRUE(store);
    EXPECT_EQ("/blob/my-test", store->getBaseBlobId());
    EXPECT_THAT(store->getBlobIds(),
                UnorderedElementsAre("/blob/my-test", "/blob/my-test/0",
                                     "/blob/my-test/1", "/blob/my-test/2",
                                     "/blob/my-test/3"));
    // Check that the storage has not changed
    EXPECT_EQ(initialData, blobDataStorage);
}

TEST_F(BinaryStoreTest, TestReadBlob)
{
    auto testDataFile = createBlobStorage(inputProto);
    auto store =
        binstore::BinaryStore::createFromFile(std::move(testDataFile), true);
    ASSERT_TRUE(store);

    const auto blobStoredData = store->readBlob("/blob/my-test/1");
    EXPECT_FALSE(blobStoredData.empty());

    decltype(blobStoredData) origData(blobData.begin(), blobData.end());

    EXPECT_THAT(blobStoredData, ElementsAreArray(origData));
}

TEST_F(BinaryStoreTest, TestReadBlobError)
{
    auto testDataFile = createBlobStorage(inputProto);
    auto store =
        binstore::BinaryStore::createFromFile(std::move(testDataFile), true);
    ASSERT_TRUE(store);

    EXPECT_THROW(store->readBlob("/nonexistent/1"), ipmi::HandlerCompletion);
}

TEST_F(BinaryStoreTest, TestOpenReadOnlyBlob)
{
    auto testDataFile = createBlobStorage(inputProto);
    auto store =
        binstore::BinaryStore::createFromFile(std::move(testDataFile), true);
    ASSERT_TRUE(store);

    EXPECT_TRUE(
        store->openOrCreateBlob("/blob/my-test/2", blobs::OpenFlags::read));
    EXPECT_FALSE(store->openOrCreateBlob(
        "/blob/my-test/2", blobs::OpenFlags::read & blobs::OpenFlags::write));
}

TEST_F(BinaryStoreTest, TestWriteExceedMaxSize)
{
    std::vector<uint8_t> writeData(10, 0);
    auto testDataFile = createBlobStorage(smallInputProto);
    auto store = binstore::BinaryStore::createFromConfig(
        "/s/test", std::move(testDataFile), 48);
    ASSERT_TRUE(store);

    EXPECT_TRUE(store->openOrCreateBlob(
        "/s/test/0", blobs::OpenFlags::write | blobs::OpenFlags::read));
    // Current size 24(blob_ + max_size) + 8(size var) = 32
    EXPECT_TRUE(store->write(
        0, writeData)); // 44 =  32(existing) + 10 (data) + 2 (blob_id '/0')
    EXPECT_FALSE(
        store->write(10, writeData)); // 54 = 44 (existing) + 10 (new data)
    EXPECT_FALSE(
        store->write(5, writeData)); // 49 = 44 (existing) + 5 (new data)
    EXPECT_TRUE(
        store->write(4, writeData)); // 48 = 44 (existing) + 4 (new data)
}

TEST_F(BinaryStoreTest, TestCreateFromConfigExceedMaxSize)
{
    auto testDataFile = createBlobStorage(inputProto);
    auto store = binstore::BinaryStore::createFromConfig(
        "/blob/my-test", std::move(testDataFile), 1);
    ASSERT_TRUE(store);
    EXPECT_FALSE(store->commit());
}

TEST_F(BinaryStoreTest, TestCreateFromFileExceedMaxSize)
{
    auto testDataFile = createBlobStorage(inputProto);
    auto store = binstore::BinaryStore::createFromFile(std::move(testDataFile),
                                                       false, 1);

    // Reading from File is expected to call loadSerializedData and fail at the
    // commit()
    EXPECT_FALSE(store);
}
