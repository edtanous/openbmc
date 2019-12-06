#pragma once

#include <register/hei_register.hpp>

namespace libhei
{

class NotRegister : public Register
{
  public:
    NotRegister(Register& i_arg) :
        Register(), iv_child(&i_arg), iv_iBS(i_arg.GetBitLength())
    {}

    NotRegister& operator=(const NotRegister& r)
    {
        iv_child = r.iv_child;
        iv_iBS   = r.iv_iBS;
        return *this;
    }

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
    LeftShiftRegister(Register& i_arg, uint16_t i_amount) :
        Register(), iv_child(&i_arg), iv_amount(i_amount),
        iv_iBS(i_arg.GetBitLength())
    {}

    LeftShiftRegister& operator=(const LeftShiftRegister& r)
    {
        iv_child  = r.iv_child;
        iv_amount = r.iv_amount;
        iv_iBS    = r.iv_iBS;
        return *this;
    }

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
    RightShiftRegister(Register& i_arg, uint16_t i_amount) :
        Register(), iv_child(&i_arg), iv_amount(i_amount),
        iv_iBS(i_arg.GetBitLength())
    {}

    RightShiftRegister& operator=(const RightShiftRegister& r)
    {
        iv_child  = r.iv_child;
        iv_amount = r.iv_amount;
        iv_iBS    = r.iv_iBS;
        return *this;
    }

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
    AndRegister(Register& i_left, Register& i_right) :
        Register(), iv_left(&i_left), iv_right(&i_right),
        iv_iBS(std::min(i_left.GetBitLength(), i_right.GetBitLength()))
    {}

    AndRegister& operator=(const AndRegister& r)
    {
        iv_left  = r.iv_left;
        iv_right = r.iv_right;
        iv_iBS   = r.iv_iBS;
        return *this;
    }

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
    OrRegister(Register& i_left, Register& i_right) :
        Register(), iv_left(&i_left), iv_right(&i_right),
        iv_iBS(std::min(i_left.GetBitLength(), i_right.GetBitLength()))
    {}

    OrRegister& operator=(const OrRegister& r)
    {
        iv_left  = r.iv_left;
        iv_right = r.iv_right;
        iv_iBS   = r.iv_iBS;
        return *this;
    }

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
    ConstantRegister(const BitStringBuffer& i_arg) : Register(), iv_iBS(i_arg)
    {}

    ConstantRegister& operator=(const ConstantRegister& r)
    {
        iv_iBS = r.iv_iBS;
        return *this;
    }

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
