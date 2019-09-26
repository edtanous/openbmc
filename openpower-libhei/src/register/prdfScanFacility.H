/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/diag/prdf/common/framework/register/prdfScanFacility.H $ */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2018                        */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

#ifndef PRDFSCANFACILITY_H
#define PRDFSCANFACILITY_H
/**
   @file prdfScanFacility.H
   @brief Description
*/


//--------------------------------------------------------------------
// Includes
//--------------------------------------------------------------------

#include <register/hei_hardware_register.hpp>
#include <register/hei_operator_register.hpp>
#include <util/hei_flyweight.hpp>

#include <vector>
#include <prdfPlatServices.H>

namespace libhei
{


/**
    PRD Scan Facility
    @author Doug Gilbert
    @par    The Scan facility is used by PRD to access Scan and Scan Comm
    @       functions.It attempts to reduce duplicate objects and their
    @       aggragates as must as possible.
*/
class ScanFacility
{
public:
  /**
   The Scan Facility is a singleton - this function provides access to it.
   */
  static ScanFacility & Access(void);

    /**
    * @brief     Returns reference to flyweight object of type HardwareRegister.
    *            An object of given address is first searched in flyweight.
    *            If object exist, reference to existing object is returned
    *            else a new one is created.
    * @param     i_address        address of the register
    * @param     i_scomLength     length of the bit string
    * @param     i_type           type of target associated with register
    * @param     i_regOp          operations supported for given register
    * @return   returns reference to flyweight object from factory
    */
   Register & GetScanCommRegister( uint64_t address,
                                uint32_t i_scomLength, TARGETING::TYPE i_type,
                                Register::AccessLevel i_regOp );

  /**
   * @brief  Get a register that bitwise inverts the bitstring of a register
   *         when read or written to
   * @param  Register source
   * @post   Only one instance of the register with this SCR parameter will
   *         exist
   */
  Register & GetNotRegister(Register & i_arg);

  /**
   * @brief  Get a register that bitwise left shift the bitstring of a register
   *         when read or written to
   * @param  Register source
   * @post   Only one instance of the register with this SCR parameter and
   *         amount will exist
   */
  Register & GetLeftShiftRegister(
                        Register & i_arg, uint16_t i_amount);

  /**
   * @brief Get a register that bitwise right shift the bitstring of a register
   *        when read or written to
   * @param Register source
   * @post  Only one instance of the register with this SCR parameter and amount
   * @      will exist
   */
  Register & GetRightShiftRegister(
                        Register & i_arg, uint16_t i_amount);

  /**
   * @brief  Get a register for the summary construct
   * @param  Register source
   * @param  uint16_t i_bit  bit to set if any attentions found in i_arg
   * @post   Only one instance of the register with this SCR parameter and
   *         amount will exist
   */
  Register & GetSummaryRegister(
                        Register & i_arg, uint16_t i_bit);

  /**
   * @brief  Get a register that bit-wise ANDs the bitstring of two register
   *         when read or written to
   * @param  The 2 SCR 's to AND
   * @posrt  Only one instance of the register with these SCRs will exist
   */
  Register & GetAndRegister(Register & i_left,
                                            Register & i_right);

  /**
   * @brief  Get a register that bitwise ORs the bitstrings of two register when
   *         read or written
   * @param  the 2 SCR's to OR
   * @post   Only one instance of the register with these SCR's will exist
   */
  Register & GetOrRegister(Register & i_left,
                                           Register & i_right);

  /**
   * @brief  Get a AttnTypeRegister
   * @params 5 pointers to scr Registers
   * @post only one instance of the register with these SCR's will exist
   */
  Register & GetAttnTypeRegister(
                                    Register * i_check,
                                    Register * i_recov,
                                    Register * i_special,
                                    Register * i_proccs,
                                    Register * i_hostattn );

  /**
   * @brief Get a PrdfConstantRegister
   * @param BitString - the bit string constant to use.
   * @post only one instance of the register with this BIT_STRING value will
   * exist.
   */
  Register & GetConstantRegister(const BitStringBuffer & i_val);
  /**
   * @brief Get a plugin  register
   * @param Reference to target less flyweight object
   * @param RuleChip  associatd with register
   * @post only one instance of the register with this BIT_STRING value will
   * exist.
   */
  Register & GetPluginRegister(
                                Register & i_flyweight,
                                ExtensibleChip & i_RuleChip );
  /**
   * @brief Delete all the plugin  register
   * @param None
   * @post  all the ScomRegisterAccess register flyweight object created for
   *        plugin shall be deleted
   * exist.
   */
  void ResetPluginRegister();

  /**
   * @brief Intended to reset all the flyweights if PRD is uninitialized due to
   *        a reIPL, reset/reload, or failover. This free up the memory and
   *        avoids memory leaks in the flyweights.
   */
  void reset();

#ifdef FLYWEIGHT_PROFILING
/**
 * @brief       prints memory allocated for object residing on flyweight
 */
  void printStats();
#endif

  /**
   Destructor
   */
//  ~ScanFacility();
private:  // functions
  /**
   Constructor
   * @param
   * @returns
   * @pre
   * @post
   * @see
   * @note
   */
  ScanFacility() {}


private:  // Data
  typedef Flyweight<HardwareRegister>           ScanCommRegisters;
  typedef Flyweight<AttnTypeRegister>        AttnTypeRegisters;
  typedef Flyweight<AndRegister>            AndRegisters;
  typedef Flyweight<OrRegister>             OrRegisters;
  typedef Flyweight<NotRegister>            NotRegisters;
  typedef Flyweight<LeftShiftRegister>      LeftShiftRegisters;
  typedef Flyweight<RightShiftRegister>    RightShiftRegisters;
  typedef Flyweight<SummaryRegister>        SummaryRegisters;
  typedef Flyweight<ConstantRegister>       ConstantRegisters;
  typedef Flyweight<ScomRegisterAccess>    PluginRegisters;

  ScanCommRegisters     iv_scomRegFw;
  AttnTypeRegisters     iv_attnRegFw;
  AndRegisters          iv_andRegFw;
  OrRegisters           iv_orRegFw;
  NotRegisters          iv_notRegFw;
  LeftShiftRegisters    iv_leftRegFw;
  SummaryRegisters      iv_sumRegFw;
  RightShiftRegisters   iv_rightRegFw;
  ConstantRegisters     iv_constRegFw;
  PluginRegisters       iv_pluginRegFw;

};

} // end namespace libhei

#endif /* PRDFSCANFACILITY_H */
