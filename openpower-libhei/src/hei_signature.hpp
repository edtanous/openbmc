#pragma once

#include <hei_chip.hpp>
#include <hei_types.hpp>

namespace libhei
{

/**
 * @brief A signature represents an active attention from a single bit position
 *        from an isolation node within a chip.
 *
 * The isolator will gather a list of all active attentions on a chip and return
 * a list of signatures to the user application. The user application should be
 * able to determine what actions to take based on these signatures.
 */
class Signature
{
  public: // Constructors, destructor, assignment, etc.
    /**
     * @brief Default contructor.
     *
     * In general the default constructor should not be used, but it is needed
     * for some STL functions.
     */
    Signature() = default;

    /**
     * @brief Constructor from components.
     * @param i_chip     The chip containing this register.
     * @param i_id       The node ID.
     * @param i_instance The instance of this register.
     * @param i_bit      The target bit within this register.
     * @param i_attnType The attention type reported by this bit.
     */
    Signature(const Chip& i_chip, NodeId_t i_id, Instance_t i_instance,
              BitPosition_t i_bit, AttentionType_t i_attnType) :
        iv_chip(i_chip),
        iv_id(i_id), iv_instance(i_instance), iv_bit(i_bit),
        iv_attnType(i_attnType)
    {}

    /** @brief Destructor. */
    ~Signature() = default;

    /** @brief Copy constructor. */
    Signature(const Signature&) = default;

    /** @brief Assignment operator. */
    Signature& operator=(const Signature&) = default;

  private:
    Chip iv_chip;                ///< Chip containing this register.
    NodeId_t iv_id;              ///< Node ID.
    Instance_t iv_instance;      ///< Instance of this register.
    BitPosition_t iv_bit;        ///< Target bit within this register.
    AttentionType_t iv_attnType; ///< Attention type reported by this bit.

  public: // Member functions
    /** @return The chip containing this register. */
    const Chip& getChip() const
    {
        return iv_chip;
    }

    /** @return The node ID. */
    NodeId_t getId() const
    {
        return iv_id;
    }

    /** @return The instance of this register. */
    Instance_t getInstance() const
    {
        return iv_instance;
    }

    /** @return The target bit within this register. */
    BitPosition_t getBit() const
    {
        return iv_bit;
    }

    /** @return The attention type reported by this bit. */
    AttentionType_t getAttnType() const
    {
        return iv_attnType;
    }

    /** @return A 32-bit integer containing the node ID, instance, and bit. This
     *          should be helpful for consistent logging and tracing. */
    uint32_t toUint32() const
    {
        return getId() << 16 | getInstance() << 8 | getBit();
    }

  public: // Operators
    /** @brief Equals operator. */
    bool operator==(const Signature& i_r) const
    {
        return (getChip() == i_r.getChip() && getId() == i_r.getId() &&
                getInstance() == i_r.getInstance() &&
                getBit() == i_r.getBit() && getAttnType() == i_r.getAttnType());
    }

    /** @brief Less than operator. */
    bool operator<(const Signature& i_r) const
    {
        if (getChip() < i_r.getChip())
        {
            return true;
        }
        else if (getChip() == i_r.getChip())
        {
            if (getId() < i_r.getId())
            {
                return true;
            }
            else if (getId() == i_r.getId())
            {
                if (getInstance() < i_r.getInstance())
                {
                    return true;
                }
                else if (getInstance() == i_r.getInstance())
                {
                    if (getBit() < i_r.getBit())
                    {
                        return true;
                    }
                    else if (getBit() == i_r.getBit())
                    {
                        return (getAttnType() < i_r.getAttnType());
                    }
                }
            }
        }

        return false;
    }
};

} // end namespace libhei
