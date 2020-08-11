#include <isolator/hei_isolation_node.hpp>
#include <util/hei_bit_string.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

bool IsolationNode::analyze(const Chip& i_chip, AttentionType_t i_attnType,
                            IsolationData& io_isoData) const
{
    bool o_activeAttn = false; // Initially, assume no active attentions.

    // Keep track of nodes that have been analyzed to avoid cyclic isolation.
    pushIsolationStack();

    // Capture all registers for this node.
    for (const auto& hwReg : iv_capRegs)
    {
        // Read the register (adds BitString to register cache).
        if (hwReg->read(i_chip))
        {
            // TODO: Would be nice to add SCOM errors to the log just in case
            //       traces are not available.
            // TODO: This trace could be redundant with the user application,
            //       which will have more information on the actual chip that
            //       failed anyway. Leaving it commented out for now until the
            //       SCOM errors are added to the log.
            // HEI_ERR("register read failed on chip type=0x%0" PRIx32
            //         "address=0x%0" PRIx64,
            //         i_chip.getType(), hwReg->getAddress());
        }

        // TODO: Add this register to io_isoData.
        // TODO: getBitString() does read hardware if read() has not been called
        //       and there is nothing in the register cache. However, it does
        //       not does not indicate if the read was successful. Not sure if
        //       this is intentional. Will need to investigate.
        // auto bs = hwReg->getBitString();
    }

    // A rule for i_attnType must exist.
    auto rule_itr = iv_rules.find(i_attnType);
    HEI_ASSERT(iv_rules.end() != rule_itr);

    // Get the returned BitString for this rule.
    const BitString* bs = rule_itr->second->getBitString(i_chip);

    // Ensure this BitString is not longer than the maximum bit field.
    HEI_ASSERT(bs->getBitLen() <= (1 << (sizeof(BitPosition_t) * 8)));

    // Find all active bits for this rule.
    for (BitPosition_t bit = 0; bit < bs->getBitLen(); bit++)
    {
        // Continue to the next bit if not active.
        if (!bs->isBitSet(bit))
            continue;

        // At least one active bit was found.
        o_activeAttn = true;

        // Determine if this attention originated from another register or if it
        // is a leaf in the isolation tree.
        auto child_itr = iv_children.find(bit);
        if (iv_children.end() != child_itr)
        {
            // This bit was driven from an attention from another register.
            // Continue down the isolation tree to look for more attentions.
            bool attnFound =
                child_itr->second->analyze(i_chip, i_attnType, io_isoData);
            if (!attnFound)
            {
                // Something went wrong. There should have been an active
                // attention. It's possible there is a bug in the Chip Data
                // File. Or, it is also possible some other piece of code is
                // clearing the attention before this code is able to analyze
                // it. Another possibility is that the hardware it not behaving
                // as expected. Since we really don't know what happened, we
                // should not assert. Instead, add this bit's signature to
                // io_isoData. If there are no other active attentions, the user
                // application could use this signature to help determine, and
                // circumvent, the isolation problem.
                io_isoData.addSignature(
                    Signature{i_chip, iv_id, iv_instance, bit, i_attnType});
            }
        }
        else
        {
            // We have reached a leaf in the isolation tree. Add this bit's
            // signature to io_isoData.
            io_isoData.addSignature(
                Signature{i_chip, iv_id, iv_instance, bit, i_attnType});
        }
    }

    // Analysis is complete on this node. So remove it from cv_isolationStack.
    popIsolationStack();

    return o_activeAttn;
}

//------------------------------------------------------------------------------

void IsolationNode::addCaptureRegister(HardwareRegister::ConstPtr i_hwReg)
{
    HEI_ASSERT(i_hwReg); // should not be null

    // If the register already exists, ignore it. Otherwise, add it to the list.
    auto itr = std::find(iv_capRegs.begin(), iv_capRegs.end(), i_hwReg);
    if (iv_capRegs.end() == itr)
    {
        iv_capRegs.push_back(i_hwReg);
    }
}

//------------------------------------------------------------------------------

void IsolationNode::addRule(AttentionType_t i_attnType,
                            Register::ConstPtr i_rule)
{
    HEI_ASSERT(i_rule); // should not be null

    auto ret = iv_rules.emplace(i_attnType, i_rule);

    // If an entry already existed, it must point to the same object.
    HEI_ASSERT(ret.second || (ret.first->second == i_rule));
}

//------------------------------------------------------------------------------

void IsolationNode::addChild(uint8_t i_bit, ConstPtr i_child)
{
    HEI_ASSERT(i_child); // should not be null

    auto ret = iv_children.emplace(i_bit, i_child);

    // If an entry already existed, it must point to the same object.
    HEI_ASSERT(ret.second || (ret.first->second == i_child));
}

//------------------------------------------------------------------------------

std::vector<const IsolationNode*> IsolationNode::cv_isolationStack{};

//------------------------------------------------------------------------------

void IsolationNode::pushIsolationStack() const
{
    // Ensure this node does not already exist in cv_isolationStack.
    auto itr =
        std::find(cv_isolationStack.begin(), cv_isolationStack.end(), this);
    HEI_ASSERT(cv_isolationStack.end() == itr);

    // Push to node to the stack.
    cv_isolationStack.push_back(this);
}

//------------------------------------------------------------------------------

} // end namespace libhei
