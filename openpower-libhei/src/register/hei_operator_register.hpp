#pragma once

#include <register/hei_register.hpp>

/**
 * @file hei_operator_register.hpp
 *
 * A library of useful classes used to perform logical or bitwise math on or
 * between other registers. The classes implemented here include NotRegister,
 * LeftShiftRegister, RightShiftRegister, AndRegister, OrRegister, and
 * ConstantRegister.
 *
 * Accompanied with other Register classes and the getBitString() function, it
 * is possible to perform operations like:
 *
 *      AndRegister reg{<register1>,<register2>};
 *      result = reg.getBitString(<someChip>);
 *
 * This example will return a BitString containing the result of the bitwise
 * AND operation applied to register1 and register2.
 */
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
    size_t getSize() const override
    {
        return (BitString::getMinBytes(iv_result.getBitLen()));
    }
};

// Pure virtual destructor must be defined.
inline OperatorRegister::~OperatorRegister() {}

/**
 * @brief Using getBitString(), performs a bitwise NOT operation on a register.
 *
 * Example:
 *      NotRegister reg{someRegister};
 *      result = reg.getBitString(someChip);
 */
class NotRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_arg Target register for operation.
     */
    explicit NotRegister(Register::ConstPtr i_arg) :
        OperatorRegister(i_arg->getSize()), iv_child(i_arg)
    {}

    /** @brief Destructor. */
    ~NotRegister() = default;

    /** @brief Copy constructor. */
    NotRegister(const NotRegister&) = delete;

    /** @brief Assignment operator. */
    NotRegister& operator=(const NotRegister& r) = delete;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip& i_chip) const override
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
    const Register::ConstPtr iv_child;
};

/**
 * @brief Using getBitString(), performs a left shift operation on a register.
 *
 * Example:
 *      LeftShiftRegister reg{someRegister1, shiftValue};
 *      result = reg.getBitString(someChip);
 */
class LeftShiftRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_arg    Target register for operation.
     * @param i_amount The shift value.
     */
    LeftShiftRegister(Register::ConstPtr i_arg, size_t i_amount) :
        OperatorRegister(i_arg->getSize()), iv_child(i_arg), iv_amount(i_amount)
    {}

    /** @brief Destructor. */
    ~LeftShiftRegister() = default;

    /** @brief Copy constructor. */
    LeftShiftRegister(const LeftShiftRegister&) = delete;

    /** @brief Assignment operator. */
    LeftShiftRegister& operator=(const LeftShiftRegister& r) = delete;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip& i_chip) const override
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
    const Register::ConstPtr iv_child;
    const size_t iv_amount;
};

/**
 * @brief Using getBitString(), performs a right shift operation on a register.
 *
 * Example:
 *      RightShiftRegister reg{someRegister1, shiftValue};
 *      result = reg.getBitString(someChip);
 */
class RightShiftRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_arg    Target register for operation.
     * @param i_amount The shift value.
     */
    RightShiftRegister(Register::ConstPtr i_arg, size_t i_amount) :
        OperatorRegister(i_arg->getSize()), iv_child(i_arg), iv_amount(i_amount)
    {}

    /** @brief Destructor. */
    ~RightShiftRegister() = default;

    /** @brief Copy constructor. */
    RightShiftRegister(const RightShiftRegister&) = delete;

    /** @brief Assignment operator. */
    RightShiftRegister& operator=(const RightShiftRegister& r) = delete;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip& i_chip) const override
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
    const Register::ConstPtr iv_child;
    const size_t iv_amount;
};

/**
 * @brief Using getBitString(), performs a bitwise AND operation on a pair
 *        of registers.
 *
 * Example:
 *      AndRegister reg{someRegister1, someRegister2};
 *      result = reg.getBitString(someChip);
 */
class AndRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_left  Target register for operation.
     * @param i_right Target register for operation.
     */
    AndRegister(Register::ConstPtr i_left, Register::ConstPtr i_right) :
        OperatorRegister(i_left->getSize()), iv_left(i_left), iv_right(i_right)
    {
        // The two registers must be the same sizes or it makes for some weird
        // results.
        HEI_ASSERT(iv_left->getSize() == iv_right->getSize());
    }

    /** @brief Destructor. */
    ~AndRegister() = default;

    /** @brief Copy constructor. */
    AndRegister(const AndRegister&) = delete;

    /** @brief Assignment operator. */
    AndRegister& operator=(const AndRegister& r) = delete;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip& i_chip) const override
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
    const Register::ConstPtr iv_left;
    const Register::ConstPtr iv_right;
};

/**
 * @brief Using getBitString(), performs a bitwise OR operation on a pair
 *        of registers.
 *
 * Example:
 *      OrRegister reg{someRegister1, someRegister2};
 *      result = reg.getBitString(someChip);
 */
class OrRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_left  Target register for operation.
     * @param i_right Target register for operation.
     */
    OrRegister(Register::ConstPtr i_left, Register::ConstPtr i_right) :
        OperatorRegister(i_left->getSize()), iv_left(i_left), iv_right(i_right)
    {
        // The two registers must be the same sizes or it makes for some weird
        // results.
        HEI_ASSERT(iv_left->getSize() == iv_right->getSize());
    }

    /** @brief Destructor. */
    ~OrRegister() = default;

    /** @brief Copy constructor. */
    OrRegister(const OrRegister&) = delete;

    /** @brief Assignment operator. */
    OrRegister& operator=(const OrRegister& r) = delete;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip& i_chip) const override
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
    const Register::ConstPtr iv_left;
    const Register::ConstPtr iv_right;
};

/**
 * @brief Contains a constant value that can be used within any of the other
 *        register operators. The value can be retrieved using the
 *        getBitString() function.
 */
class ConstantRegister : public OperatorRegister
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_val An unsigned integer value. iv_result will be initialized to
     *              the size of type T and this value will be copied into that
     *              buffer.
     */
    template <class T>
    explicit ConstantRegister(T i_val) : OperatorRegister(sizeof(i_val))
    {
        iv_result.setFieldRight(0, iv_result.getBitLen(), i_val);
    }

    /** @brief Destructor. */
    ~ConstantRegister() = default;

    /** @brief Copy constructor. */
    ConstantRegister(const ConstantRegister&) = delete;

    /** @brief Assignment operator. */
    ConstantRegister& operator=(const ConstantRegister& r) = delete;

    /** @brief Overloaded from parent class. */
    const BitString* getBitString(const Chip&) const override
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
