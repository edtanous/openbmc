#include <hei_chip.hpp>
#include <register/hei_operator_register.hpp>
#include <util/hei_flyweight.hpp>

#include "gtest/gtest.h"

using namespace libhei;

constexpr uint16_t CONST1 = 0x1458;
constexpr uint16_t CONST2 = 0x5368;

#define MATH(x) static_cast<uint16_t>(x)

uint64_t __getVal(Register::ConstPtr i_reg)
{
    Chip CHIP{nullptr, 0};
    auto bs = i_reg->getBitString(CHIP);
    return bs->getFieldRight(0, bs->getBitLen());
}

size_t __getBW(Register::ConstPtr i_reg)
{
    Chip CHIP{nullptr, 0};
    return i_reg->getBitString(CHIP)->getBitLen();
}

TEST(OperatorRegisterTest, BasicOperations)
{
    auto const1 = std::make_shared<const ConstantRegister>(CONST1);
    ASSERT_EQ(MATH(CONST1), __getVal(const1));

    auto const2 = std::make_shared<const ConstantRegister>(CONST2);
    ASSERT_EQ(MATH(CONST2), __getVal(const2));

    auto and1 = std::make_shared<const AndRegister>(const1, const2);
    ASSERT_EQ(MATH(CONST1 & CONST2), __getVal(and1));

    auto or1 = std::make_shared<const OrRegister>(const1, const2);
    ASSERT_EQ(MATH(CONST1 | CONST2), __getVal(or1));

    auto not1 = std::make_shared<const NotRegister>(const1);
    ASSERT_EQ(MATH(~CONST1), __getVal(not1));

    auto not2 = std::make_shared<const NotRegister>(const2);
    ASSERT_EQ(MATH(~CONST2), __getVal(not2));
}

TEST(OperatorRegisterTest, BasicOperationsWithFlyweights)
{
    auto& const_factory = Flyweight<const ConstantRegister>::getSingleton();
    auto& and_factory   = Flyweight<const AndRegister>::getSingleton();
    auto& or_factory    = Flyweight<const OrRegister>::getSingleton();
    auto& not_factory   = Flyweight<const NotRegister>::getSingleton();

    auto const1 = const_factory.get(CONST1);
    auto const2 = const_factory.get(CONST2);

    ASSERT_EQ(MATH(CONST1), __getVal(const1));
    ASSERT_EQ(MATH(CONST2), __getVal(const2));
    ASSERT_EQ(MATH(CONST1 & CONST2), __getVal(and_factory.get(const1, const2)));
    ASSERT_EQ(MATH(CONST1 | CONST2), __getVal(or_factory.get(const1, const2)));
    ASSERT_EQ(MATH(~CONST1), __getVal(not_factory.get(const1)));
    ASSERT_EQ(MATH(~CONST2), __getVal(not_factory.get(const2)));

    const_factory.clear();
    and_factory.clear();
    or_factory.clear();
    not_factory.clear();
}

TEST(OperatorRegisterTest, ShiftOperations)
{
    auto& const_factory  = Flyweight<const ConstantRegister>::getSingleton();
    auto& lshift_factory = Flyweight<const LeftShiftRegister>::getSingleton();
    auto& rshift_factory = Flyweight<const RightShiftRegister>::getSingleton();

    auto const1 = const_factory.get(CONST1);

    for (size_t i = 0; i < __getBW(const1); i++)
    {
        ASSERT_EQ(MATH(CONST1 << i), __getVal(lshift_factory.get(const1, i)));
    }

    for (size_t i = 0; i < __getBW(const1); i++)
    {
        ASSERT_EQ(MATH(CONST1 >> i), __getVal(rshift_factory.get(const1, i)));
    }

    const_factory.clear();
    lshift_factory.clear();
    rshift_factory.clear();
}

TEST(OperatorRegisterTest, ComplexOperation)
{
    // Something seemingly complex:
    //   ~(((CONST1 & CONST2) << 12) | ((CONST1 | CONST2) >> 4)))

    auto& const_factory  = Flyweight<const ConstantRegister>::getSingleton();
    auto& and_factory    = Flyweight<const AndRegister>::getSingleton();
    auto& or_factory     = Flyweight<const OrRegister>::getSingleton();
    auto& not_factory    = Flyweight<const NotRegister>::getSingleton();
    auto& lshift_factory = Flyweight<const LeftShiftRegister>::getSingleton();
    auto& rshift_factory = Flyweight<const RightShiftRegister>::getSingleton();

    auto const1 = const_factory.get(CONST1);
    auto const2 = const_factory.get(CONST2);

    auto lshift = lshift_factory.get(and_factory.get(const1, const2), 12);
    auto rshift = rshift_factory.get(or_factory.get(const1, const2), 4);

    auto expr = not_factory.get(or_factory.get(lshift, rshift));

    ASSERT_EQ(MATH(~(((CONST1 & CONST2) << 12) | ((CONST1 | CONST2) >> 4))),
              __getVal(expr));

    const_factory.clear();
    and_factory.clear();
    or_factory.clear();
    not_factory.clear();
    lshift_factory.clear();
    rshift_factory.clear();
}

TEST(OperatorRegisterTest, ConstRegConstructor)
{
    auto& const_factory = Flyweight<const ConstantRegister>::getSingleton();
    auto& and_factory   = Flyweight<const AndRegister>::getSingleton();

    // The ConstRegister constructor is a template that requires some integer
    // type.

    auto const1 = const_factory.get(CONST1); // CONST1 is uint16_t, GOOD

    auto const2 = const_factory.get(uint16_t{0x1458}); // This also works

    ASSERT_EQ(const1, const2); // Should point to the same thing.

    // If you pass in a constant with no type the compiler assumes it is the
    // implicit int type. This value is the same as const2, but the size will be
    // different.
    auto const3 = const_factory.get(0x1458);

    // The values are the same, but the bit strings are are different sizes.
    ASSERT_NE(const2, const3);

    // AND and OR operators will not like different sizes either.
    ASSERT_DEATH(and_factory.get(const2, const3), "");

    const_factory.clear();
    and_factory.clear();
}
