#pragma once

#include "sys_file.hpp"

#include <unistd.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "binaryblob.pb.h"

using std::size_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;

namespace binstore
{

/**
 * @class BinaryStoreInterface is an abstraction for a storage location.
 *     Each instance would be uniquely identified by a baseId string.
 */
class BinaryStoreInterface
{
  public:
    virtual ~BinaryStoreInterface() = default;

    /**
     * @returns baseId string of the storage.
     */
    virtual std::string getBaseBlobId() const = 0;

    /**
     * @returns List of all open blob IDs, plus the base.
     */
    virtual std::vector<std::string> getBlobIds() const = 0;

    /**
     * Opens a blob given its name. If there is no one, create one.
     * @param blobId: The blob id to operate on.
     * @param flags: Either read flag or r/w flag has to be specified.
     * @returns True if open/create successfully.
     */
    virtual bool openOrCreateBlob(const std::string& blobId,
                                  uint16_t flags) = 0;

    /**
     * Deletes a blob given its name. If there is no one,
     * @param blobId: The blob id to operate on.
     * @returns True if deleted.
     */
    virtual bool deleteBlob(const std::string& blobId) = 0;

    /**
     * Reads data from the currently opened blob.
     * @param offset: offset into the blob to read
     * @param requestedSize: how many bytes to read
     * @returns Bytes able to read. Returns empty if nothing can be read or
     *          if there is no open blob.
     */
    virtual std::vector<uint8_t> read(uint32_t offset,
                                      uint32_t requestedSize) = 0;

    /**
     * Writes data to the currently openend blob.
     * @param offset: offset into the blob to write
     * @param data: bytes to write
     * @returns True if able to write the entire data successfully
     */
    virtual bool write(uint32_t offset, const std::vector<uint8_t>& data) = 0;

    /**
     * Commits data to the persistent storage specified during blob init.
     * @returns True if able to write data to sysfile successfully
     */
    virtual bool commit() = 0;

    /**
     * Closes blob, which prevents further modifications. Uncommitted data will
     * be lost.
     * @returns True if able to close the blob successfully
     */
    virtual bool close() = 0;

    /**
     * TODO
     */
    virtual bool stat() = 0;
};

/**
 * @class BinaryStore instantiates a concrete implementation of
 *     BinaryStoreInterface. The dependency on file is injected through its
 *     constructor.
 */
class BinaryStore : public BinaryStoreInterface
{
  public:
    BinaryStore() = delete;
    BinaryStore(const std::string& baseBlobId, std::unique_ptr<SysFile> file,
                uint32_t maxSize) :
        baseBlobId_(baseBlobId),
        file_(std::move(file)), maxSize_(maxSize)
    {
        blob_.set_blob_base_id(baseBlobId_);
    }

    ~BinaryStore() = default;

    BinaryStore(const BinaryStore&) = delete;
    BinaryStore& operator=(const BinaryStore&) = delete;
    BinaryStore(BinaryStore&&) = default;
    BinaryStore& operator=(BinaryStore&&) = default;

    std::string getBaseBlobId() const override;
    std::vector<std::string> getBlobIds() const override;
    bool openOrCreateBlob(const std::string& blobId, uint16_t flags) override;
    bool deleteBlob(const std::string& blobId) override;
    std::vector<uint8_t> read(uint32_t offset, uint32_t requestedSize) override;
    bool write(uint32_t offset, const std::vector<uint8_t>& data) override;
    bool commit() override;
    bool close() override;
    bool stat() override;

    /**
     * Helper factory method to create a BinaryStore instance
     * @param baseBlobId: base id for the created instance
     * @param sysFile: system file object for storing binary
     * @param maxSize: max size in bytes that this BinaryStore can expand to.
     *     Writing data more than allowed size will return failure.
     * @returns unique_ptr to constructed BinaryStore. Caller should take
     *     ownership of the instance.
     */
    static std::unique_ptr<BinaryStoreInterface>
        createFromConfig(const std::string& baseBlobId,
                         std::unique_ptr<SysFile> file, uint32_t maxSize);

  private:
    enum class CommitState
    {
        Dirty,         // In-memory data might not match persisted data
        Clean,         // In-memory data matches persisted data
        Uninitialized, // Cannot find persisted data
        CommitError    // Error happened during committing
    };

    /* Load the serialized data from sysfile if commit state is dirty.
     * Returns False if encountered error when loading */
    bool loadSerializedData();

    std::string baseBlobId_;
    binaryblobproto::BinaryBlobBase blob_;
    binaryblobproto::BinaryBlob* currentBlob_ = nullptr;
    bool writable_ = false;
    std::unique_ptr<SysFile> file_ = nullptr;
    uint32_t maxSize_;
    CommitState commitState_ = CommitState::Dirty;
};

} // namespace binstore
