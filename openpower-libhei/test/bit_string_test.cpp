#include <hei_user_defines.hpp>
#include <util/hei_bit_string.hpp>
#include "gtest/gtest.h"

using namespace libhei;

static constexpr uint32_t UINT8_BIT_LEN = (sizeof(uint8_t) * 8);
static constexpr uint32_t UINT64_BIT_LEN = (sizeof(uint64_t) * 8);

// setBit()
// clearBit()
// setAll()
// clearAll()
// isBitSet()
// getSetCount()
// isZero()
TEST( BitStringTest, TestSet1 )
{
    BitStringBuffer bs(UINT64_BIT_LEN);
    uint32_t i;

    // set all bits in ascending order
    for(i = 0; i < UINT64_BIT_LEN; i++)
    {
        // Make sure bit gets set and set count
        // is increasing
        bs.setBit(i);
        ASSERT_TRUE(bs.isBitSet(i));
        ASSERT_EQ(bs.getSetCount(), i+1);
    }
    // all bits should be set at this point
    ASSERT_EQ(bs.getFieldRight(0,64), UINT64_MAX);
    ASSERT_EQ(bs.getFieldLeft(0,64), UINT64_MAX);

    // test clearAll(), setAll()
    bs.clearAll();
    ASSERT_TRUE(bs.isZero());
    bs.setAll();
    ASSERT_EQ(bs.getSetCount(), UINT64_BIT_LEN);

    // clear all bits in descending order
    for(i = UINT64_BIT_LEN; 0 != i; i--)
    {
        // make sure bit gets cleared and set count
        // is decreasing
        ASSERT_EQ(bs.getSetCount(), i);
        bs.clearBit(i-1);
        ASSERT_FALSE(bs.isBitSet(i-1));
    }
    // all bits should be clear at this point
    ASSERT_EQ(bs.getSetCount(), 0u);
    ASSERT_EQ(bs.getFieldRight(0,64), 0u);
    ASSERT_EQ(bs.getFieldLeft(0,64), 0u);
    ASSERT_TRUE(bs.isZero());
}

// setPattern()
TEST( BitStringTest, TestSet2 )
{
    BitStringBuffer bs(UINT64_BIT_LEN);
    uint64_t field = 0xaaaaaaaaaaaaaaaa;

    bs.setPattern(field);
    ASSERT_EQ(field, bs.getFieldRight(0, 64));
    ASSERT_EQ(field, bs.getFieldLeft(0, 64));

    bs.clearAll();
    ASSERT_TRUE(bs.isZero());

    bs.setPattern(0xaa, 8);
    ASSERT_EQ(field, bs.getFieldRight(0, 64));
    ASSERT_EQ(field, bs.getFieldLeft(0, 64));

    bs.clearAll();
    ASSERT_TRUE(bs.isZero());

    bs.setPattern(0, 64, 0xaaaa, 16);
    ASSERT_EQ(field, bs.getFieldRight(0, 64));
    ASSERT_EQ(field, bs.getFieldLeft(0, 64));
}

// setString()
TEST( BitStringTest, TestSet3 )
{
    BitStringBuffer bsb_dest(64);
    BitStringBuffer bsb_src(64);

    bsb_dest.clearAll();
    ASSERT_TRUE(bsb_dest.isZero());

    bsb_src.setAll();
    ASSERT_EQ(64u, bsb_src.getSetCount());

    bsb_dest.setString(bsb_src);
    ASSERT_FALSE(bsb_dest.isZero());
    ASSERT_EQ(bsb_dest.getFieldRight(0, 64), bsb_src.getFieldRight(0,64));
    ASSERT_EQ(bsb_dest.getFieldLeft(0, 64), bsb_src.getFieldLeft(0,64));
}

// maskString()
TEST( BitStringTest, TestSet4 )
{
    BitStringBuffer bsb(64);
    bsb.setAll();
    ASSERT_EQ(64u, bsb.getSetCount());

    BitStringBuffer bsb_mask(64);
    bsb_mask.setFieldRight(0, 64, 0xaaaaaaaaaaaaaaaa);

    bsb.maskString(bsb_mask);
    ASSERT_EQ(bsb.getFieldRight(0, 64), 0x5555555555555555u);
}

// setFieldRight()
// setFieldLeft()
// getFielRight()
// getFieldLeft()
TEST( BitStringTest, TestSet5 )
{
    uint64_t field = 0x1234567890abcdef;
    BitStringBuffer bsb(64);

    // set bitstring to low end of field
    bsb.setFieldRight(0, 32, field);
    ASSERT_EQ(field << 32, bsb.getFieldLeft(0,32));

    // set bitstring to high end of field
    bsb.setFieldLeft(0, 32, field);
    ASSERT_EQ(field >> 32, bsb.getFieldRight(0,32));

    // increasing offset
    for(uint32_t i = 0; i < UINT64_BIT_LEN; i++)
    {
        // increasing length
        for(uint32_t j = 1; j <= UINT64_BIT_LEN - i; j++)
        {
            bsb.clearAll();
            bsb.setFieldRight(i, j, UINT64_MAX);

            // verify
            ASSERT_EQ(bsb.getFieldRight(i, j), \
                        UINT64_MAX >> (UINT64_BIT_LEN - j));

        }
    }
    for(uint32_t i = 0; i < UINT64_BIT_LEN; i++)
    {
        // set 1 bit at offset i
        bsb.clearAll();
        bsb.setFieldRight(i, 1, 1);

        // verify bit is set
        ASSERT_EQ(bsb.getFieldRight(0, 64), (uint64_t)1 << \
                                    (UINT64_BIT_LEN - i - 1));
    }
}

// operator >>
// operator <<
TEST( BitStringTest, TestSet6 )
{
    uint64_t field = 0x1234567890abcdef;
    BitStringBuffer bsb(64);

    bsb.setFieldRight(0, 64, field);
    ASSERT_EQ(field, bsb.getFieldRight(0, 64));
    ASSERT_EQ(field >> 32, (bsb >> 32).getFieldRight(0, 64));
    ASSERT_EQ(field << 32, (bsb << 32).getFieldRight(0, 64));
}
