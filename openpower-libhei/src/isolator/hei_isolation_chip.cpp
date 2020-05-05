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

} // end namespace libhei
