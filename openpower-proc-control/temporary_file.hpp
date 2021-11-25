#pragma once

#include <filesystem>
#include <utility>

namespace openpower::util
{

namespace fs = std::filesystem;

/**
 * @class TemporaryFile
 *
 * A temporary file in the file system.
 *
 * The temporary file is created by the constructor.  The absolute path to the
 * file can be obtained using getPath().
 *
 * The temporary file can be deleted by calling remove().  Otherwise the file
 * will be deleted by the destructor.
 *
 */
class TemporaryFile
{
  public:
    // Specify which compiler-generated methods we want
    TemporaryFile(const TemporaryFile&) = delete;
    TemporaryFile(TemporaryFile&&) = delete;
    TemporaryFile& operator=(const TemporaryFile&) = delete;

    /**
     * Constructor.
     *
     * Creates a temporary file in the temporary directory (normally /tmp).
     *
     * Throws an exception if the file cannot be created.
     */
    TemporaryFile();

    /**
     * Destructor.
     *
     * Deletes the temporary file if necessary.
     */
    ~TemporaryFile()
    {
        try
        {
            remove();
        }
        catch (...)
        {
            // Destructors should not throw exceptions
        }
    }

    /**
     * Deletes the temporary file.
     *
     * Does nothing if the file has already been deleted.
     *
     * Log error message if an error occurs during the deletion.
     */
    void remove();

    /**
     * Returns the absolute path to the temporary file.
     *
     * Returns an empty path if the file has been deleted.
     *
     * @return temporary file path
     */
    const fs::path& getPath() const
    {
        return path;
    }

  private:
    /**
     * Absolute path to the temporary file.
     *
     * Empty when file has been deleted.
     */
    fs::path path{};
};

} // namespace openpower::util
