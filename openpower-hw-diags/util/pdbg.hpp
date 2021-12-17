#pragma once

#include <libpdbg.h>

#include <analyzer/callout.hpp>

#include <string>
#include <vector>

// Forward reference to avoid pulling the libhei library into everything that
// includes this header.
namespace libhei
{
class Chip;
}

namespace util
{

namespace pdbg
{

/** Chip target types. */
enum TargetType_t : uint8_t
{
    TYPE_PROC   = 0x05,
    TYPE_IOLINK = 0x47,
    TYPE_OMI    = 0x48,
    TYPE_OCMB   = 0x4b,
    TYPE_IOHS   = 0x51,
};

/** @return The target associated with the given chip. */
pdbg_target* getTrgt(const libhei::Chip& i_chip);

/** @return The target associated with the given devtree path. */
pdbg_target* getTrgt(const std::string& i_path);

/** @return A string representing the given target's devtree path. */
const char* getPath(pdbg_target* i_trgt);

/** @return A string representing the given chip's devtree path. */
const char* getPath(const libhei::Chip& i_chip);

/** @return The absolute position of the given target. */
uint32_t getChipPos(pdbg_target* i_trgt);

/** @return The absolute position of the given chip. */
uint32_t getChipPos(const libhei::Chip& i_chip);

/** @return The target type of the given target. */
uint8_t getTrgtType(pdbg_target* i_trgt);

/** @return The target type of the given chip. */
uint8_t getTrgtType(const libhei::Chip& i_chip);

/**
 * @return The connected target on the other side of the given bus.
 * @param  i_rxTarget The target on the receiving side (RX) of the bus.
 * @param  i_busType  The bus type.
 */
pdbg_target* getConnectedTarget(pdbg_target* i_rxTarget,
                                const analyzer::callout::BusType& i_busType);

/**
 * @return The pib target associated with the given proc target.
 * @note   Will assert the given target is a proc target.
 * @note   Will assert the returned pib target it not nullptr.
 */
pdbg_target* getPibTrgt(pdbg_target* i_procTrgt);

/**
 * @return The fsi target associated with the given proc target.
 * @note   Will assert the given target is a proc target.
 * @note   Will assert the returned fsi target it not nullptr.
 */
pdbg_target* getFsiTrgt(pdbg_target* i_procTrgt);

/**
 * @brief  Reads a SCOM register.
 * @param  i_trgt Given target.
 * @param  i_addr Given address.
 * @param  o_val  The returned value of the register.
 * @return 0 if successful, non-0 otherwise.
 * @note   Will assert the given target is a proc target.
 */
int getScom(pdbg_target* i_trgt, uint64_t i_addr, uint64_t& o_val);

/**
 * @brief  Reads a CFAM FSI register.
 * @param  i_trgt Given target.
 * @param  i_addr Given address.
 * @param  o_val  The returned value of the register.
 * @return 0 if successful, non-0 otherwise.
 * @note   Will assert the given target is a proc target.
 */
int getCfam(pdbg_target* i_trgt, uint32_t i_addr, uint32_t& o_val);

/**
 * @brief Returns the list of all active chips in the system.
 * @param o_chips The returned list of chips.
 */
void getActiveChips(std::vector<libhei::Chip>& o_chips);

/**
 * @return The primary processor (i.e. the processor connected to the BMC).
 */
pdbg_target* getPrimaryProcessor();

/**
 * @return True, if hardware analysis is supported on this system. False,
 *         otherwise.
 * @note   Support for hardware analysis from the BMC started with P10 systems
 *         and is not supported on any older chip generations.
 */
bool queryHardwareAnalysisSupported();

/**
 * @return A string containing the FRU location code of the given chip. An empty
 *         string indicates the target was null or the attribute does not exist
 *         for this target.
 * @note   This function requires PHAL APIs that are only available in certain
 *         environments. If they do not exist the devtree path of the target is
 *         returned.
 */
std::string getLocationCode(pdbg_target* trgt);

/**
 * @return A string containing the physical device path (entity path) of the
 *         given chip. An empty string indicates the target was null or the
 *         attribute does not exist for this target.
 * @note   This function requires PHAL APIs that are only available in certain
 *         environments. If they do not exist the devtree path of the target is
 *         returned.
 */
std::string getPhysDevPath(pdbg_target* trgt);

/**
 * @return A vector of bytes representing the numerical values of the physical
 *         device path (entity path) of the given target. An empty vector
 *         indicates the target was null or the attribute does not exist for
 *         this target or any parent targets along the device tree path.
 * @note   This function requires PHAL APIs that are only available in certain
 *         environments. If they do not exist, an empty vector is returned.
 */
std::vector<uint8_t> getPhysBinPath(pdbg_target* trgt);

/**
 * @brief  Uses an SBE chip-op to query if there has been an LPC timeout.
 * @return True, if there was an LPC timeout. False, otherwise.
 */
bool queryLpcTimeout(pdbg_target* target);

} // namespace pdbg

} // namespace util
