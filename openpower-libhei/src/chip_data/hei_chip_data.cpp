#include <chip_data/hei_chip_data.hpp>
#include <chip_data/hei_chip_data_stream.hpp>
#include <register/hei_scom_register.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

using FileKeyword_t = uint64_t;

constexpr FileKeyword_t KW_CHIPDATA = 0x4348495044415441; // "CHIPDATA" ASCII

using SectionKeyword_t = uint32_t;

constexpr SectionKeyword_t KW_REGS = 0x52454753; // "REGS" ASCII
constexpr SectionKeyword_t KW_NODE = 0x4e4f4445; // "NODE" ASCII
constexpr SectionKeyword_t KW_ROOT = 0x524f4f54; // "ROOT" ASCII

using Version_t = uint8_t;

constexpr Version_t VERSION_1 = 0x01;

//------------------------------------------------------------------------------

void __readRegister(ChipDataStream& io_stream, IsolationChipPtr& io_isoChip)
{
    // Read the register metadata.
    RegisterId_t id;
    RegisterType_t type;
    RegisterAttributeFlags_t attr;
    Instance_t numInsts;
    io_stream >> id >> type >> attr >> numInsts;

    // Must have at least one instance.
    HEI_ASSERT(0 != numInsts);

    for (Instance_t i = 0; i < numInsts; i++)
    {
        // Read the register instance metadata.
        Instance_t inst;
        io_stream >> inst;

        // The address size is dependent on the register type.
        if (REG_TYPE_SCOM == type)
        {
            uint32_t addr; // 4-byte address.
            io_stream >> addr;

            // Get this register from the flyweight factory.
            auto& factory = Flyweight<const ScomRegister>::getSingleton();
            HardwareRegisterPtr hwReg = factory.get(id, inst, attr, addr);

            // Add this register to the isolation chip.
            io_isoChip->addHardwareRegister(hwReg);
        }
        else if (REG_TYPE_ID_SCOM == type)
        {
            uint64_t addr; // 8-byte address.
            io_stream >> addr;

            // Get this register from the flyweight factory.
            auto& factory = Flyweight<const IdScomRegister>::getSingleton();
            HardwareRegisterPtr hwReg = factory.get(id, inst, attr, addr);

            // Add this register to the isolation chip.
            io_isoChip->addHardwareRegister(hwReg);
        }
        else
        {
            HEI_ASSERT(false); // Register type unsupported.
        }
    }
}

//------------------------------------------------------------------------------

void parseChipDataFile(void* i_buffer, size_t i_bufferSize,
                       IsolationChipMap& io_isoChips)
{
    ChipDataStream stream{i_buffer, i_bufferSize};

    // Read the file metadata.
    FileKeyword_t fileKeyword;
    ChipType_t chipType;
    Version_t version;
    stream >> fileKeyword >> chipType >> version;

    // Check the file keyword.
    HEI_ASSERT(KW_CHIPDATA == fileKeyword);

    // This chip type should not already exist.
    HEI_ASSERT(io_isoChips.end() == io_isoChips.find(chipType));

    // So far there is only one supported version type so check it here.
    HEI_ASSERT(VERSION_1 == version);

    // Allocate memory for the new isolation chip.
    auto isoChip = std::make_unique<IsolationChip>(chipType);

    // Read the register list metadata.
    SectionKeyword_t regsKeyword;
    RegisterId_t numRegs;
    stream >> regsKeyword >> numRegs;

    // Check the register keyword.
    HEI_ASSERT(KW_REGS == regsKeyword);

    // There must be at least one register defined.
    HEI_ASSERT(0 != numRegs);

    for (uint32_t i = 0; i < numRegs; i++)
    {
        __readRegister(stream, isoChip);
    }

    // Add this isolation chip to the collective list of isolation chips.
    auto ret = io_isoChips.emplace(chipType, std::move(isoChip));
    HEI_ASSERT(ret.second); // Just in case.
}

//------------------------------------------------------------------------------

} // namespace libhei
