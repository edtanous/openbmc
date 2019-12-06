#pragma once

#include <register/hei_register.hpp>

namespace libhei
{

/**
 * @brief An abstract class for all operator registers.
 *
 * Contains member functions and variables that are required for all child
 * classes.
 */
class OperatorRegister : public Register
{
  public:
    /** @brief Pure virtual destructor. */
    virtual ~OperatorRegister() = 0;

  protected:
    /**
     * @brief Constructor from components.
     * @param i_size Size (in bytes) of this register.
     */
    explicit OperatorRegister(size_t i_size) : Register(), iv_result(i_size * 8)
    {}

  protected: // Instance variables
    /** When getBitString() is called on an operator, the resulting value of the
     *  operation will be stored in this instance variable. */
    BitStringBuffer iv_result;

  public:
    /** @brief Overloaded from parent class. */
    virtual const BitString* getBitString(const Chip& i_chip) const = 0;

    /** @brief Overloaded from parent class. */
    size_t getSize() const
    {
        return (BitString::getMinBytes(iv_result.getBitLen()));
    }
};

// Pure virtual destructor must be defined.
inline OperatorRegister::~OperatorRegister() {}

class NotRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_arg Target register for operation.
     */
    explicit NotRegister(Register& i_arg) :
        OperatorRegister(i_arg.getSize()), iv_child(&i_arg)
    {}

    /** @brief Default destructor. */
    ~NotRegister() = default;

    /** @brief Default copy constructor. */
    NotRegister(const NotRegister&) = default;

    /** @brief Default assignment operator. */
    NotRegister& operator=(const NotRegister& r) = default;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip& i_chip) const
    {
        const auto* bs = iv_child->getBitString(i_chip);

        (const_cast<NotRegister*>(this))->iv_result = ~(*bs);

        return &iv_result;
    }

    /** @brief Comparison operator. */
    bool operator==(const NotRegister& r) const
    {
        return iv_child == r.iv_child;
    }

    /** @brief Less-than operator. */
    bool operator<(const NotRegister& r) const
    {
        return iv_child < r.iv_child;
    }

  private:
    Register* iv_child;
};

class LeftShiftRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_arg    Target register for operation.
     * @param i_amount The shift value.
     */
    LeftShiftRegister(Register& i_arg, uint16_t i_amount) :
        OperatorRegister(i_arg.getSize()), iv_child(&i_arg), iv_amount(i_amount)
    {}

    /** @brief Default destructor. */
    ~LeftShiftRegister() = default;

    /** @brief Default copy constructor. */
    LeftShiftRegister(const LeftShiftRegister&) = default;

    /** @brief Default assignment operator. */
    LeftShiftRegister& operator=(const LeftShiftRegister& r) = default;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip& i_chip) const
    {
        const auto* bs = iv_child->getBitString(i_chip);

        (const_cast<LeftShiftRegister*>(this))->iv_result = (*bs) << iv_amount;

        return &iv_result;
    }

    /** @brief Comparison operator. */
    bool operator==(const LeftShiftRegister& r) const
    {
        return (iv_child == r.iv_child) && (iv_amount == r.iv_amount);
    }

    /** @brief Less-than operator. */
    bool operator<(const LeftShiftRegister& r) const
    {
        if (iv_child == r.iv_child)
            return iv_amount < r.iv_amount;
        return iv_child < r.iv_child;
    }

  private:
    Register* iv_child;
    uint16_t iv_amount;
};

class RightShiftRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_arg    Target register for operation.
     * @param i_amount The shift value.
     */
    RightShiftRegister(Register& i_arg, uint16_t i_amount) :
        OperatorRegister(i_arg.getSize()), iv_child(&i_arg), iv_amount(i_amount)
    {}

    /** @brief Default destructor. */
    ~RightShiftRegister() = default;

    /** @brief Default copy constructor. */
    RightShiftRegister(const RightShiftRegister&) = default;

    /** @brief Default assignment operator. */
    RightShiftRegister& operator=(const RightShiftRegister& r) = default;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip& i_chip) const
    {
        const auto* bs = iv_child->getBitString(i_chip);

        (const_cast<RightShiftRegister*>(this))->iv_result = (*bs) >> iv_amount;

        return &iv_result;
    }

    /** @brief Comparison operator. */
    bool operator==(const RightShiftRegister& r) const
    {
        return (iv_child == r.iv_child) && (iv_amount == r.iv_amount);
    }

    /** @brief Less-than operator. */
    bool operator<(const RightShiftRegister& r) const
    {
        if (iv_child == r.iv_child)
            return iv_amount < r.iv_amount;
        return iv_child < r.iv_child;
    }

  private:
    Register* iv_child;
    uint16_t iv_amount;
};

class AndRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_left  Target register for operation.
     * @param i_right Target register for operation.
     */
    AndRegister(Register& i_left, Register& i_right) :
        OperatorRegister(std::min(i_left.getSize(), i_right.getSize())),
        iv_left(&i_left), iv_right(&i_right)
    {}

    /** @brief Default destructor. */
    ~AndRegister() = default;

    /** @brief Default copy constructor. */
    AndRegister(const AndRegister&) = default;

    /** @brief Default assignment operator. */
    AndRegister& operator=(const AndRegister& r) = default;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip& i_chip) const
    {
        const auto* l_bs = iv_left->getBitString(i_chip);
        const auto* r_bs = iv_right->getBitString(i_chip);

        (const_cast<AndRegister*>(this))->iv_result = (*l_bs) & (*r_bs);

        return &iv_result;
    }

    /** @brief Comparison operator. */
    bool operator==(const AndRegister& r) const
    {
        return (iv_left == r.iv_left) && (iv_right == r.iv_right);
    }

    /** @brief Less-than operator. */
    bool operator<(const AndRegister& r) const
    {
        if (iv_left == r.iv_left)
            return iv_right < r.iv_right;
        return iv_left < r.iv_left;
    }

  private:
    Register* iv_left;
    Register* iv_right;
};

class OrRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_left  Target register for operation.
     * @param i_right Target register for operation.
     */
    OrRegister(Register& i_left, Register& i_right) :
        OperatorRegister(std::min(i_left.getSize(), i_right.getSize())),
        iv_left(&i_left), iv_right(&i_right)
    {}

    /** @brief Default destructor. */
    ~OrRegister() = default;

    /** @brief Default copy constructor. */
    OrRegister(const OrRegister&) = default;

    /** @brief Default assignment operator. */
    OrRegister& operator=(const OrRegister& r) = default;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip& i_chip) const
    {
        const auto* l_bs = iv_left->getBitString(i_chip);
        const auto* r_bs = iv_right->getBitString(i_chip);

        (const_cast<OrRegister*>(this))->iv_result = (*l_bs) | (*r_bs);

        return &iv_result;
    }

    /** @brief Comparison operator. */
    bool operator==(const OrRegister& r) const
    {
        return (iv_left == r.iv_left) && (iv_right == r.iv_right);
    }

    /** @brief Less-than operator. */
    bool operator<(const OrRegister& r) const
    {
        if (iv_left == r.iv_left)
            return iv_right < r.iv_right;
        return iv_left < r.iv_left;
    }

  private:
    Register* iv_left;
    Register* iv_right;
};

class ConstantRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_arg A BitStringBuffer containing the constant value.
     */
    explicit ConstantRegister(const BitStringBuffer& i_arg) :
        OperatorRegister(BitString::getMinBytes(i_arg.getBitLen()))
    {
        iv_result = i_arg;
    }

    /** @brief Default destructor. */
    ~ConstantRegister() = default;

    /** @brief Default copy constructor. */
    ConstantRegister(const ConstantRegister&) = default;

    /** @brief Default assignment operator. */
    ConstantRegister& operator=(const ConstantRegister& r) = default;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip& i_chip) const
    {
        return &iv_result;
    }

    /** @brief Comparison operator. */
    bool operator==(const ConstantRegister& r) const
    {
        return iv_result == r.iv_result;
    }

    /** @brief Less-than operator. */
    bool operator<(const ConstantRegister& r) const
    {
        return iv_result < r.iv_result;
    }
};

} // end namespace libhei
