#pragma once

#include <register/hei_register.hpp>

namespace libhei
{

class NotRegister : public Register
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_arg Target register for operation.
     */
    explicit NotRegister(Register& i_arg) :
        Register(), iv_child(&i_arg), iv_iBS(i_arg.GetBitLength())
    {}

    /** @brief Default destructor. */
    ~NotRegister() = default;

    /** @brief Default copy constructor. */
    NotRegister(const NotRegister&) = default;

    /** @brief Default assignment operator. */
    NotRegister& operator=(const NotRegister& r) = default;

    const BitString* getBitString(const Chip& i_chip) const
    {
        const auto* bs = iv_child->getBitString(i_chip);

        (const_cast<NotRegister*>(this))->iv_iBS = ~(*bs);

        return &iv_iBS;
    }

    bool operator==(const NotRegister& r) const
    {
        return r.iv_child == iv_child;
    }

    bool operator<(const NotRegister& r) const
    {
        return iv_child < r.iv_child;
    }

  private:
    Register* iv_child;

    BitStringBuffer iv_iBS;
};

class LeftShiftRegister : public Register
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_arg    Target register for operation.
     * @param i_amount The shift value.
     */
    LeftShiftRegister(Register& i_arg, uint16_t i_amount) :
        Register(), iv_child(&i_arg), iv_amount(i_amount),
        iv_iBS(i_arg.GetBitLength())
    {}

    /** @brief Default destructor. */
    ~LeftShiftRegister() = default;

    /** @brief Default copy constructor. */
    LeftShiftRegister(const LeftShiftRegister&) = default;

    /** @brief Default assignment operator. */
    LeftShiftRegister& operator=(const LeftShiftRegister& r) = default;

    const BitString* getBitString(const Chip& i_chip) const
    {
        const auto* bs = iv_child->getBitString(i_chip);

        (const_cast<LeftShiftRegister*>(this))->iv_iBS = (*bs) << iv_amount;

        return &iv_iBS;
    }

    bool operator==(const LeftShiftRegister& r) const
    {
        return (r.iv_child == iv_child) && (r.iv_amount == iv_amount);
    }

    bool operator<(const LeftShiftRegister& r) const
    {
        if (iv_child == r.iv_child)
            return iv_amount < r.iv_amount;
        return iv_child < r.iv_child;
    }

  private:
    Register* iv_child;
    uint16_t iv_amount;

    BitStringBuffer iv_iBS;
};

class RightShiftRegister : public Register
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_arg    Target register for operation.
     * @param i_amount The shift value.
     */
    RightShiftRegister(Register& i_arg, uint16_t i_amount) :
        Register(), iv_child(&i_arg), iv_amount(i_amount),
        iv_iBS(i_arg.GetBitLength())
    {}

    /** @brief Default destructor. */
    ~RightShiftRegister() = default;

    /** @brief Default copy constructor. */
    RightShiftRegister(const RightShiftRegister&) = default;

    /** @brief Default assignment operator. */
    RightShiftRegister& operator=(const RightShiftRegister& r) = default;

    const BitString* getBitString(const Chip& i_chip) const
    {
        const auto* bs = iv_child->getBitString(i_chip);

        (const_cast<RightShiftRegister*>(this))->iv_iBS = (*bs) >> iv_amount;

        return &iv_iBS;
    }

    bool operator==(const RightShiftRegister& r) const
    {
        return (r.iv_child == iv_child) && (r.iv_amount == iv_amount);
    }

    bool operator<(const RightShiftRegister& r) const
    {
        if (iv_child == r.iv_child)
            return iv_amount < r.iv_amount;
        return iv_child < r.iv_child;
    }

  private:
    Register* iv_child;
    uint16_t iv_amount;

    BitStringBuffer iv_iBS;
};

class AndRegister : public Register
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_left  Target register for operation.
     * @param i_right Target register for operation.
     */
    AndRegister(Register& i_left, Register& i_right) :
        Register(), iv_left(&i_left), iv_right(&i_right),
        iv_iBS(std::min(i_left.GetBitLength(), i_right.GetBitLength()))
    {}

    /** @brief Default destructor. */
    ~AndRegister() = default;

    /** @brief Default copy constructor. */
    AndRegister(const AndRegister&) = default;

    /** @brief Default assignment operator. */
    AndRegister& operator=(const AndRegister& r) = default;

    const BitString* getBitString(const Chip& i_chip) const
    {
        const auto* l_bs = iv_left->getBitString(i_chip);
        const auto* r_bs = iv_right->getBitString(i_chip);

        (const_cast<AndRegister*>(this))->iv_iBS = (*l_bs) & (*r_bs);

        return &iv_iBS;
    }

    bool operator==(const AndRegister& r) const
    {
        return (r.iv_left == iv_left) && (r.iv_right == iv_right);
    }

    bool operator<(const AndRegister& r) const
    {
        if (iv_left == r.iv_left)
            return iv_right < r.iv_right;
        return iv_left < r.iv_left;
    }

  private:
    Register* iv_left;
    Register* iv_right;

    BitStringBuffer iv_iBS;
};

class OrRegister : public Register
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_left  Target register for operation.
     * @param i_right Target register for operation.
     */
    OrRegister(Register& i_left, Register& i_right) :
        Register(), iv_left(&i_left), iv_right(&i_right),
        iv_iBS(std::min(i_left.GetBitLength(), i_right.GetBitLength()))
    {}

    /** @brief Default destructor. */
    ~OrRegister() = default;

    /** @brief Default copy constructor. */
    OrRegister(const OrRegister&) = default;

    /** @brief Default assignment operator. */
    OrRegister& operator=(const OrRegister& r) = default;

    const BitString* getBitString(const Chip& i_chip) const
    {
        const auto* l_bs = iv_left->getBitString(i_chip);
        const auto* r_bs = iv_right->getBitString(i_chip);

        (const_cast<OrRegister*>(this))->iv_iBS = (*l_bs) | (*r_bs);

        return &iv_iBS;
    }

    bool operator==(const OrRegister& r) const
    {
        return (r.iv_left == iv_left) && (r.iv_right == iv_right);
    }

    bool operator<(const OrRegister& r) const
    {
        if (iv_left == r.iv_left)
            return iv_right < r.iv_right;
        return iv_left < r.iv_left;
    }

  private:
    Register* iv_left;
    Register* iv_right;

    BitStringBuffer iv_iBS;
};

class ConstantRegister : public Register
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_arg A BitStringBuffer containing the constant value.
     */
    explicit ConstantRegister(const BitStringBuffer& i_arg) :
        Register(), iv_iBS(i_arg)
    {}

    /** @brief Default destructor. */
    ~ConstantRegister() = default;

    /** @brief Default copy constructor. */
    ConstantRegister(const ConstantRegister&) = default;

    /** @brief Default assignment operator. */
    ConstantRegister& operator=(const ConstantRegister& r) = default;

    const BitString* getBitString(const Chip& i_chip) const
    {
        return &iv_iBS;
    }

    bool operator==(const ConstantRegister& r) const
    {
        return r.iv_iBS == iv_iBS;
    }

  private:
    BitStringBuffer iv_iBS;
};

} // end namespace libhei
