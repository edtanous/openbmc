#include <chip_data/hei_chip_data.hpp>
#include <chip_data/hei_chip_data_stream.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

using FileKeyword_t = uint64_t;

constexpr FileKeyword_t KW_CHIPDATA = 0x4348495044415441; // "CHIPDATA" ASCII

using SectionKeyword_t = uint32_t;

constexpr SectionKeyword_t KW_REGS = 0x43484950; // "REGS" ASCII
constexpr SectionKeyword_t KW_NODE = 0x4e4f4445; // "NODE" ASCII
constexpr SectionKeyword_t KW_ROOT = 0x524f4f54; // "ROOT" ASCII

using Version_t = uint8_t;

constexpr Version_t VERSION_1 = 0x01;

//------------------------------------------------------------------------------

void parseChipDataFile(void* i_buffer, size_t i_bufferSize,
                       IsolationChipMap& io_isoChips)
{
    ChipDataStream stream{i_buffer, i_bufferSize};

    // Read the file metadata.
    FileKeyword_t file_kw = 0;
    ChipType_t chipType   = 0;
    Version_t version     = 0;
    stream >> file_kw >> chipType >> version;

    // Check the file ID.
    HEI_ASSERT(KW_CHIPDATA == file_kw);

    // This chip type should not already exist.
    HEI_ASSERT(io_isoChips.end() == io_isoChips.find(chipType));

    // Allocate memory for the new isolation chip.
    auto isoChip = std::make_unique<IsolationChip>(chipType);

    // TODO

    // Add this isolation chip to the collective list of isolation chips.
    auto ret = io_isoChips.emplace(chipType, std::move(isoChip));
    HEI_ASSERT(ret.second); // Just in case.
}

//------------------------------------------------------------------------------

} // namespace libhei
