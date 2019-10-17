#pragma once

#include <hei_includes.hpp>

namespace libhei
{

/**
 * @brief A signature represents an active attention from a single bit on a
 *        register within a chip.
 *
 * The isolator will gather a list of all active attentions on a chip and return
 * a list of signatures to the user application. The user application should be
 * able to determine what actions to take based on these signatures.
 */
class Signature
{
  public: // Constructors, destructor, assignment, etc.

    /**
     * @brief Constructor from components.
     * @param i_chip     The chip containing this register.
     * @param i_id       The register ID.
     * @param i_instance The instance of this register.
     * @param i_bit      The target bit within this register.
     * @param i_attnType The attention type reported by this bit.
     */
    Signature( const Chip & i_chip, RegisterId_t i_id,
               RegisterInstance_t i_instance, RegisterBit_t i_bit,
               AttentionType_t i_attnType ) :
        iv_chip( i_chip ), iv_id( i_id ), iv_instance( i_instance ),
        iv_bit( i_bit ), iv_attnType( i_attnType )
    {}

    /** @brief Destructor. */
    ~Signature() = default;

    /** @brief Copy constructor. */
    Signature( const Signature & ) = default;

    /** @brief Assignment operator. */
    Signature & operator=( const Signature & ) = default;

  private: // Instance variables.

    Chip               iv_chip;     ///< Chip containing this register.
    RegisterId_t       iv_id;       ///< Register ID.
    RegisterInstance_t iv_instance; ///< Instance of this register.
    RegisterBit_t      iv_bit;      ///< Target bit within this register.
    AttentionType_t    iv_attnType; ///< Attention type reported by this bit.

  public: // Member functions

    /** @return The chip containing this register. */
    const Chip & getChip() const { return iv_chip; }

    /** @return The register ID. */
    RegisterId_t getId() const { return iv_id; }

    /** @return The instance of this register. */
    RegisterInstance_t getInstance() const { return iv_instance; }

    /** @return The target bit within this register. */
    RegisterBit_t getBit() const { return iv_bit; }

    /** @return The attention type reported by this bit. */
    AttentionType_t getAttnType() const { return iv_attnType; }
};

} // end namespace libhei

