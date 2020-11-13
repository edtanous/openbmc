
#include <assert.h>

#include <hei_main.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <vector>

namespace fs = std::filesystem;

namespace analyzer
{

//------------------------------------------------------------------------------

void __getChipDataFiles(std::map<libhei::ChipType_t, fs::path>& o_files)
{
    o_files.clear();

    auto directory = "/usr/share/openpower-libhei/";

    for (const auto& entry : fs::directory_iterator(directory))
    {
        auto path = entry.path();

        std::ifstream file{path, std::ios::binary};
        if (!file.good())
        {
            trace::err("Unable to open file: %s", path.string().c_str());
            continue;
        }

        // The first 8-bytes is the file keyword and the next 4-bytes is the
        // chip type.
        libhei::FileKeyword_t keyword;
        libhei::ChipType_t chipType;

        const size_t sz_keyword  = sizeof(keyword);
        const size_t sz_chipType = sizeof(chipType);
        const size_t sz_buffer   = sz_keyword + sz_chipType;

        // Read the keyword and chip type from the file.
        char buffer[sz_buffer];
        file.read(buffer, sz_buffer);
        if (!file.good())
        {
            trace::err("Unable to read file: %s", path.string().c_str());
            continue;
        }

        // Get the keyword.
        memcpy(&keyword, &buffer[0], sz_keyword);
        keyword = be64toh(keyword);

        // Ensure the keyword value is correct.
        if (libhei::KW_CHIPDATA != keyword)
        {
            trace::err("Invalid chip data file: %s", path.string().c_str());
            continue;
        }

        // Get the chip type.
        memcpy(&chipType, &buffer[sz_keyword], sz_chipType);
        chipType = be32toh(chipType);

        // Trace each legitimate chip data file for debug.
        trace::inf("File found: type=0x%0" PRIx32 " path=%s", chipType,
                   path.string().c_str());

        // So far, so good. Add the entry.
        auto ret = o_files.emplace(chipType, path);
        assert(ret.second); // Should not have duplicate entries
    }
}

//------------------------------------------------------------------------------

void __initialize(const fs::path& i_path)
{
    // Get file size.
    const auto sz_buffer = fs::file_size(i_path);

    // Create a buffer large enough to hold the entire file.
    std::vector<char> buffer(sz_buffer);

    // Open the chip data file.
    std::ifstream file{i_path, std::ios::binary};
    assert(file.good()); // We've opened it once before, so it should open now.

    // Read the entire file into the buffer.
    file.read(buffer.data(), sz_buffer);
    assert(file.good()); // Again, this should be readable.

    // This is not necessary, but it frees up memory before calling the memory
    // intensive initialize() function.
    file.close();

    // Initialize the isolator with this chip data file.
    libhei::initialize(buffer.data(), sz_buffer);
}

//------------------------------------------------------------------------------

void initializeIsolator(std::vector<libhei::Chip>& o_chips)
{
    // Get all of the active chips to be analyzed.
    util::pdbg::getActiveChips(o_chips);

    // Find all of the existing chip data files.
    std::map<libhei::ChipType_t, fs::path> files;
    __getChipDataFiles(files);

    // Keep track of models/levels that have already been initialized.
    std::map<libhei::ChipType_t, unsigned int> initTypes;

    for (const auto& chip : o_chips)
    {
        auto chipType = chip.getType();

        // Mark this chip type as initialized (or will be if it hasn't been).
        auto ret = initTypes.emplace(chipType, 1);
        if (!ret.second)
        {
            // This type has already been initialized. Nothing more to do.
            continue;
        }

        // Get the file for this chip.
        auto itr = files.find(chipType);

        // Ensure a chip data file exist for this chip.
        assert(files.end() != itr);

        // Initialize this chip type.
        __initialize(itr->second);
    }
}

//------------------------------------------------------------------------------

} // namespace analyzer
