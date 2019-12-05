#pragma once

#include <prdrCommon.H>

#include <register/hei_register.hpp>

#include <algorithm>

namespace libhei
{

class NotRegister : public Register
{
  public:
    NotRegister() : Register(), iv_child(nullptr), iv_iBS(0)
    {
        iv_bs = &iv_iBS;
    }

    NotRegister(Register& i_arg) :
        Register(), iv_child(&i_arg), iv_iBS(i_arg.GetBitLength())
    {
        iv_bs = &iv_iBS;
    }

    NotRegister& operator=(const NotRegister& r)
    {
        iv_child = r.iv_child;
        iv_iBS   = r.iv_iBS;
        // iv_bs = r.iv_bs; <-- don't do this!
        return *this;
    }

    virtual uint32_t Read() const
    {
        return iv_child->Read();
    }
    virtual uint32_t Write()
    {
        return iv_child->Write();
    }

    const BitString* getBitString(const Chip& i_chip) const
    {
        (*iv_bs) = ~(*iv_child->getBitString(i_chip));
        return iv_bs;
    }

    virtual uint16_t GetId() const
    {
        return iv_child->GetId();
    }
    virtual void SetId(uint16_t i_id) {}

    bool operator==(const NotRegister& r) const
    {
        return r.iv_child == iv_child;
    }

    bool operator<(const NotRegister& r) const
    {
        return iv_child < r.iv_child;
    }

    bool operator>=(const NotRegister& r) const
    {
        return iv_child >= r.iv_child;
    }

  protected:
    BitString& AccessBitString(void)
    {
        return iv_iBS;
    }
    void SetBitString(const BitString*) {}

  private:
    Register* iv_child;

    BitStringBuffer* iv_bs;
    BitStringBuffer iv_iBS;
};

class LeftShiftRegister : public Register
{
  public:
    LeftShiftRegister() : Register(), iv_child(nullptr), iv_amount(0), iv_iBS(0)
    {
        iv_bs = &iv_iBS;
    }

    LeftShiftRegister(Register& i_arg, uint16_t i_amount) :
        Register(), iv_child(&i_arg), iv_amount(i_amount),
        iv_iBS(i_arg.GetBitLength())
    {
        iv_bs = &iv_iBS;
    }

    LeftShiftRegister& operator=(const LeftShiftRegister& r)
    {
        iv_child  = r.iv_child;
        iv_amount = r.iv_amount;
        iv_iBS    = r.iv_iBS;
        // iv_bs = r.iv_bs; <-- don't do this!
        return *this;
    }

    virtual uint32_t Read() const
    {
        return iv_child->Read();
    }
    virtual uint32_t Write()
    {
        return iv_child->Write();
    }

    const BitString* getBitString(const Chip& i_chip) const
    {
        (*iv_bs) = (*iv_child->getBitString(i_chip)) << iv_amount;
        return iv_bs;
    }

    virtual uint16_t GetId() const
    {
        return iv_child->GetId();
    }
    virtual void SetId(uint16_t i_id) {}

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

    bool operator>=(const LeftShiftRegister& r) const
    {
        if (iv_child == r.iv_child)
            return iv_amount >= r.iv_amount;
        return iv_child >= r.iv_child;
    }

  protected:
    BitString& AccessBitString(void)
    {
        return iv_iBS;
    }
    void SetBitString(const BitString*) {}

  private:
    Register* iv_child;
    uint16_t iv_amount;

    BitStringBuffer* iv_bs;
    BitStringBuffer iv_iBS;
};

class RightShiftRegister : public Register
{
  public:
    RightShiftRegister() :
        Register(), iv_child(nullptr), iv_amount(0), iv_iBS(0)
    {
        iv_bs = &iv_iBS;
    }

    RightShiftRegister(Register& i_arg, uint16_t i_amount) :
        Register(), iv_child(&i_arg), iv_amount(i_amount),
        iv_iBS(i_arg.GetBitLength())
    {
        iv_bs = &iv_iBS;
    }

    RightShiftRegister& operator=(const RightShiftRegister& r)
    {
        iv_child  = r.iv_child;
        iv_amount = r.iv_amount;
        iv_iBS    = r.iv_iBS;
        // iv_bs = r.iv_bs; <-- don't do this!
        return *this;
    }

    virtual uint32_t Read() const
    {
        return iv_child->Read();
    }
    virtual uint32_t Write()
    {
        return iv_child->Write();
    }

    const BitString* getBitString(const Chip& i_chip) const
    {
        (*iv_bs) = (*iv_child->getBitString(i_chip)) >> iv_amount;
        return iv_bs;
    }

    virtual uint16_t GetId() const
    {
        return iv_child->GetId();
    }
    virtual void SetId(uint16_t i_id) {}

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

    bool operator>=(const RightShiftRegister& r) const
    {
        if (iv_child == r.iv_child)
            return iv_amount >= r.iv_amount;
        return iv_child >= r.iv_child;
    }

  protected:
    BitString& AccessBitString(void)
    {
        return iv_iBS;
    }
    void SetBitString(const BitString*) {}

  private:
    Register* iv_child;
    uint16_t iv_amount;

    BitStringBuffer* iv_bs;
    BitStringBuffer iv_iBS;
};

class AndRegister : public Register
{
  public:
    AndRegister() : Register(), iv_left(nullptr), iv_right(nullptr), iv_iBS(0)
    {
        iv_bs = &iv_iBS;
    }

    AndRegister(Register& i_left, Register& i_right) :
        Register(), iv_left(&i_left), iv_right(&i_right),
        iv_iBS(std::min(i_left.GetBitLength(), i_right.GetBitLength()))
    {
        iv_bs = &iv_iBS;
    }

    AndRegister& operator=(const AndRegister& r)
    {
        iv_left  = r.iv_left;
        iv_right = r.iv_right;
        iv_iBS   = r.iv_iBS;
        // iv_bs = r.iv_bs; <-- don't do this!
        return *this;
    }

    virtual uint32_t Read() const
    {
        return iv_left->Read() | iv_right->Read();
    }
    virtual uint32_t Write()
    {
        return iv_left->Write() | iv_right->Write();
    }

    const BitString* getBitString(const Chip& i_chip) const
    {
        (*iv_bs) = *iv_left->getBitString(i_chip);
        (*iv_bs) = (*iv_bs) & (*iv_right->getBitString(i_chip));
        return iv_bs;
    }

    virtual uint16_t GetId() const
    {
        return Prdr::SignatureOp::combineSig(iv_left->GetId(),
                                             iv_right->GetId());
    }

    virtual void SetId(uint16_t i_id) {}

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

    bool operator>=(const AndRegister& r) const
    {
        if (iv_left == r.iv_left)
            return iv_right >= r.iv_right;
        return iv_left >= r.iv_left;
    }

  protected:
    BitString& AccessBitString(void)
    {
        return iv_iBS;
    }
    void SetBitString(const BitString*) {}

  private:
    Register* iv_left;
    Register* iv_right;

    BitStringBuffer* iv_bs;
    BitStringBuffer iv_iBS;
};

class OrRegister : public Register
{
  public:
    OrRegister() : Register(), iv_left(nullptr), iv_right(nullptr), iv_iBS(0)
    {
        iv_bs = &iv_iBS;
    }

    OrRegister(Register& i_left, Register& i_right) :
        Register(), iv_left(&i_left), iv_right(&i_right),
        iv_iBS(std::min(i_left.GetBitLength(), i_right.GetBitLength()))
    {
        iv_bs = &iv_iBS;
    }

    OrRegister& operator=(const OrRegister& r)
    {
        iv_left  = r.iv_left;
        iv_right = r.iv_right;
        iv_iBS   = r.iv_iBS;
        // iv_bs = r.iv_bs; <-- don't do this!
        return *this;
    }

    virtual uint32_t Read() const
    {
        return iv_left->Read() | iv_right->Read();
    }

    virtual uint32_t Write()
    {
        return iv_left->Write() | iv_right->Write();
    }

    const BitString* getBitString(const Chip& i_chip) const
    {
        (*iv_bs) = *iv_left->getBitString(i_chip);
        (*iv_bs) = (*iv_bs) | (*iv_right->getBitString(i_chip));
        return iv_bs;
    }

    virtual uint16_t GetId() const
    {
        return Prdr::SignatureOp::combineSig(iv_left->GetId(),
                                             iv_right->GetId());
    }

    virtual void SetId(uint16_t i_id) {}

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

    bool operator>=(const OrRegister& r) const
    {
        if (iv_left == r.iv_left)
            return iv_right >= r.iv_right;
        return iv_left >= r.iv_left;
    }

  protected:
    BitString& AccessBitString(void)
    {
        return iv_iBS;
    }
    void SetBitString(const BitString*) {}

  private:
    Register* iv_left;
    Register* iv_right;

    BitStringBuffer* iv_bs;
    BitStringBuffer iv_iBS;
};

class NullRegister : public Register
{
  public:
    NullRegister(int size) : Register(), iv_iBS(size) {}

    NullRegister& operator=(const NullRegister& r)
    {
        iv_iBS = r.iv_iBS;
        return *this;
    }

    virtual uint32_t Read() const
    {
        return 0;
    }
    virtual uint32_t Write()
    {
        return 0;
    }

    const BitString* getBitString(const Chip& i_chip) const
    {
        return &iv_iBS;
    }

  protected:
    BitString& AccessBitString(void)
    {
        return iv_iBS;
    }
    void SetBitString(const BitString*) {}

  private:
    BitStringBuffer iv_iBS;

    virtual uint16_t GetId() const
    {
        return Prdr::SignatureOp::DEFAULT_SIGNATURE;
    }

    virtual void SetId(uint16_t i_id) {}
};

class ConstantRegister : public Register
{
  public:
    ConstantRegister() : Register(), iv_iBS(0) {}

    ConstantRegister(const BitStringBuffer& i_arg) : Register(), iv_iBS(i_arg)
    {}

    ConstantRegister& operator=(const ConstantRegister& r)
    {
        iv_iBS = r.iv_iBS;
        return *this;
    }

    virtual uint32_t Read() const
    {
        return SUCCESS;
    }
    virtual uint32_t Write()
    {
        return SUCCESS;
    }

    const BitString* getBitString(const Chip& i_chip) const
    {
        return &iv_iBS;
    }

    virtual uint16_t GetId() const
    {
        return Prdr::SignatureOp::DEFAULT_SIGNATURE;
    }

    virtual void SetId(uint16_t i_id) {}

    bool operator==(const ConstantRegister& r) const
    {
        return r.iv_iBS == iv_iBS;
    }

  protected:
    BitString& AccessBitString(void)
    {
        return iv_iBS;
    }
    void SetBitString(const BitString*) {}

  private:
    BitStringBuffer iv_iBS;
};

} // end namespace libhei
