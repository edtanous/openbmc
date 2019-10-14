#pragma once

#include <stdio.h>
#include <hei_includes.hpp>
#include <util/hei_bit_string.hpp>

namespace libhei
{

/**
@brief Description:  The Register class.

Purpose:  Register is an abstract base class for real and virtual registers.
A few examples of these registers are; HardwareRegister, ConstantRegister,
NotRegister, and AndRegister.  As a base class of register types, Register
creates a place where these registers can be put together like logical
building blocks.  For example, Register makes this possible:

Register * fir = new HardwareRegister(REG_ADDRESS, REG_WIDTH,
                                      CHIP_TYPE, ACCESS_RO);
Register * mask = new ConstantRegister( 0xffffffff00000000 );
Register * fir_mask = new AndRegister(fir, mask);
const BitString * bs = fir_mask->getBitString();

The getBitString function (defined by each register) provides access to
the BitString that manages each register's data.  In this example bs will
contain the result of fir & mask.
*/
class Register
{
  public:

    /**
    @brief Default constructor
    */
    Register() = default;

    /**
    @brief Delete copy, assignment, and move operators.  Deleting
           copy causes the compiler to delete move.
    */
    Register(const Register&) = delete;
    Register& operator=(const Register&) = delete;

    /**
    @brief Default destructor
    */
    virtual ~Register() = default;

    /**
    @brief      Provides access to the BitString that manages
                this register's data.
    @return     A pointer to a BitString.
    */
    virtual const BitString * getBitString() const = 0;

};

}//end namespace libhei
