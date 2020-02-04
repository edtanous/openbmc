
#include <isolator/hei_isolation_node.hpp>
#include <isolator/hei_isolator.hpp>
#include <register/hei_hardware_register.hpp>
#include <util/hei_flyweight.hpp>

// BEGIN temporary code
#include <isolator/hei_isolation_node.hpp>
#include <register/hei_operator_register.hpp>
#include <register/hei_scom_register.hpp>
// END temporary code

namespace libhei
{

ReturnCode Isolator::initialize(void* i_buffer, size_t i_bufferSize,
                                bool i_forceInit)
{
    ReturnCode rc;

    // BEGIN temporary code
    HEI_INF("Isolator::initialize(%p,%" PRIu64 ",%d)", i_buffer,
            (uint64_t)i_bufferSize, i_forceInit);

    auto& scom_fw    = Flyweight<ScomRegister>::getSingleton();
    auto& idScom_fw  = Flyweight<IdScomRegister>::getSingleton();
    auto& isoNode_fw = Flyweight<IsolationNode>::getSingleton();

    auto& idScom0 = idScom_fw.get(IdScomRegister{
        static_cast<ChipType_t>(0xdeadbeef), static_cast<RegisterId_t>(0x1111),
        REG_INST_DEFAULT, REG_ACCESS_RW, 0x80000000FF000000});

    auto& scom0 = scom_fw.get(ScomRegister{
        static_cast<ChipType_t>(0xdeadbeef), static_cast<RegisterId_t>(0x2222),
        REG_INST_DEFAULT, REG_ACCESS_RW, 0x00FF0000});

    auto& node0 = isoNode_fw.get(IsolationNode{idScom0});
    auto& node1 = isoNode_fw.get(IsolationNode{scom0});

    node0.addRule(ATTN_TYPE_CHECKSTOP, &idScom0);
    node0.addChild(48, &node1);

    node1.addRule(ATTN_TYPE_CHECKSTOP, &scom0);

    iv_isoStart[static_cast<ChipType_t>(0xdeadbeef)].push_back(
        {&node0, ATTN_TYPE_CHECKSTOP});
    // END temporary code

    return rc;
}

void Isolator::uninitialize()
{
    // Remove all of the IsolationNode objects stored in the flyweights. This
    // must be done before removing the HardwareRegister objects
    Flyweight<IsolationNode>::getSingleton().clear();

    // Must flush the hardware register cache before deleting any
    // HardwareRegister objects.
    HardwareRegister::flushAll();

    // Remove all of the HardwareRegister objects stored in the flyweights.
    Flyweight<ScomRegister>::getSingleton().clear();
    Flyweight<IdScomRegister>::getSingleton().clear();
}

ReturnCode Isolator::isolate(const std::vector<Chip>& i_chipList,
                             IsolationData& o_isoData) const
{
    ReturnCode rc;

    // Flush the isolation data to ensure a clean slate.
    o_isoData.flush();

    // Flush the hardware register cache to avoid using stale data.
    HardwareRegister::flushAll();

    // Analyze active error on each chip.
    for (const auto& chip : i_chipList)
    {
        // BEGIN temporary code
        HEI_INF("Isolator::isolate(%p,%u)", chip.getChip(), chip.getType());

        // Isolation objects for this chip's type must exist.
        const auto& chip_itr = iv_isoStart.find(chip.getType());
        HEI_ASSERT(iv_isoStart.end() != chip_itr);

        for (const auto& pair : chip_itr->second)
        {
            if (pair.first->analyze(chip, pair.second, o_isoData))
            {
                for (const auto& sig : o_isoData.getSignatureList())
                {
                    HEI_INF("Signature(%p,0x%x,0x%x,0x%x,0x%x)",
                            sig.getChip().getChip(), sig.getId(),
                            sig.getInstance(), sig.getBit(), sig.getAttnType());
                }
            }
        }
        // END temporary code
    }

    return rc;
}

} // end namespace libhei
