#include <isolator/hei_isolation_chip.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

bool IsolationChip::analyze(const Chip& i_chip, IsolationData& io_isoData) const
{
    bool o_activeAttn = false; // Initially, assume no active attentions.

    // The given chip must be the same type as iv_chipType.
    HEI_ASSERT(iv_chipType == i_chip.getType());

    // Iterate each root node.
    for (const auto& node : iv_rootNodes)
    {
        if (node.second->analyze(i_chip, node.first, io_isoData))
        {
            // At least one attention type had an active attention.
            o_activeAttn = true;
        }
    }

    return o_activeAttn;
}

//------------------------------------------------------------------------------

void IsolationChip::addHardwareRegister(HardwareRegister::ConstPtr i_hwReg)
{
    HEI_ASSERT(i_hwReg); // should not be null

    auto ret = iv_regs.emplace(i_hwReg->getKey(), i_hwReg);

    // If an entry already exists, it should point to the same object.
    HEI_ASSERT(ret.second || (ret.first->second == i_hwReg));
}

//------------------------------------------------------------------------------

void IsolationChip::addIsolationNode(IsolationNode::ConstPtr i_isoNode)
{
    HEI_ASSERT(i_isoNode); // should not be null

    auto ret = iv_nodes.emplace(i_isoNode->getKey(), i_isoNode);

    // If an entry already exists, it should point to the same object.
    HEI_ASSERT(ret.second || (ret.first->second == i_isoNode));
}

//------------------------------------------------------------------------------

void IsolationChip::addRootNode(AttentionType_t i_attnType,
                                IsolationNode::ConstPtr i_rootNode)
{
    HEI_ASSERT(i_rootNode); // should not be null

    auto ret = iv_rootNodes.emplace(i_attnType, i_rootNode);

    // If an entry already exists, it should point to the same object.
    HEI_ASSERT(ret.second || (ret.first->second == i_rootNode));
}

//------------------------------------------------------------------------------

HardwareRegister::ConstPtr
    IsolationChip::getHardwareRegister(HardwareRegister::Key i_key) const
{
    auto itr = iv_regs.find(i_key);
    HEI_ASSERT(iv_regs.end() != itr); // The register should exist.

    return itr->second;
}

//------------------------------------------------------------------------------

IsolationNode::ConstPtr
    IsolationChip::getIsolationNode(IsolationNode::Key i_key) const
{
    auto itr = iv_nodes.find(i_key);
    HEI_ASSERT(iv_nodes.end() != itr); // The node should exist.

    return itr->second;
}

//------------------------------------------------------------------------------

} // end namespace libhei
