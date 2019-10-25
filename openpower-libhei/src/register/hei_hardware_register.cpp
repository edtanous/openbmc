#include <hei_includes.hpp>
#include <hei_user_interface.hpp>
#include <register/hei_hardware_register.hpp>
#include <util/hei_bit_string.hpp>

namespace libhei
{

//------------------------------------------------------------------------------

HardwareRegister::~HardwareRegister() {}

//------------------------------------------------------------------------------

#if 0
void HardwareRegister::setBitString(const BitString *bs)
{
    BitString & l_string  = accessBitString();
    l_string.setString(*bs);
}
#endif

//------------------------------------------------------------------------------

const BitString * HardwareRegister::getBitString(const Chip & i_chip) const
{
    // Verify this register belongs on i_chip.
    verifyAccessorChip(i_chip);

    // Calling read() will ensure that an entry exists in the cache and the
    // entry has at been synched with hardware at least once. Note that we
    // cannot read hardware for write-only registers. In this case, an entry
    // will be created in the cache, if it does not exist, when the cache is
    // accessed below.

    if ((REG_ACCESS_NONE != getAccessLevel()) &&
        (REG_ACCESS_WO   != getAccessLevel()))
    {
        read(i_chip);
    }

    return &(accessCache(i_chip));
}

//------------------------------------------------------------------------------

BitString & HardwareRegister::accessBitString(const Chip & i_chip)
{
    // Verify this register belongs on i_chip.
    verifyAccessorChip(i_chip);

    // Calling read() will ensure that an entry exists in the cache and the
    // entry has at been synched with hardware at least once. Note that we
    // cannot read hardware for write-only registers. In this case, an entry
    // will be created in the cache, if it does not exist, when the cache is
    // accessed below.

    if ((REG_ACCESS_NONE != getAccessLevel()) &&
        (REG_ACCESS_WO   != getAccessLevel()))
    {
        read(i_chip);
    }

    return accessCache(i_chip);
}

//------------------------------------------------------------------------------

ReturnCode HardwareRegister::read(const Chip & i_chip, bool i_force) const
{
    ReturnCode rc;

    // Verify this register belongs on i_chip.
    verifyAccessorChip(i_chip);

    // Read from hardware only if the read is forced or the entry for this
    // instance does not exist in the cache.
    if (i_force || !queryCache(i_chip))
    {
        // This register must be readable.
        HEI_ASSERT((REG_ACCESS_NONE != getAccessLevel()) &&
                   (REG_ACCESS_WO   != getAccessLevel()));

        // Get the buffer from the register cache.
        BitString & bs = accessCache(i_chip);

        // Get the byte size of the buffer.
        size_t sz_buffer = BitString::getMinBytes(bs.getBitLen());

        // Read this register from hardware.
        rc = registerRead(i_chip.getChip(), bs.getBufAddr(), sz_buffer,
                          getRegisterType(), getAddress());
        if (RC_SUCCESS != rc)
        {
            // The read failed and we can't trust what was put in the register
            // cache. So remove this instance's entry from the cache.
            flush(i_chip);
        }
        else
        {
            // Sanity check. The returned size of the data written to the buffer
            // should match the register size.
            HEI_ASSERT(getSize() == sz_buffer);
        }
    }

    return rc;
}

//------------------------------------------------------------------------------

#ifndef __HEI_READ_ONLY

ReturnCode HardwareRegister::write(const Chip & i_chip) const
{
    ReturnCode rc;

    // Verify this register belongs on i_chip.
    verifyAccessorChip(i_chip);

    // This register must be writable.
    HEI_ASSERT((REG_ACCESS_NONE != getAccessLevel()) &&
               (REG_ACCESS_RO   != getAccessLevel()));

    // An entry for this register must exist in the cache.
    HEI_ASSERT(queryCache(i_chip));

    // Get the buffer from the register cache.
    BitString & bs = accessCache(i_chip);

    // Get the byte size of the buffer.
    size_t sz_buffer = BitString::getMinBytes(bs.getBitLen());

    // Write to this register to hardware.
    rc = registerWrite(i_chip.getChip(), bs.getBufAddr(),
                       sz_buffer, getRegisterType(), getAddress());

    if (RC_SUCCESS == rc)
    {
        // Sanity check. The returned size of the data written to the buffer
        // should match the register size.
        HEI_ASSERT(getSize() == sz_buffer);
    }

    return rc;
}

#endif // __HEI_READ_ONLY

//------------------------------------------------------------------------------

HardwareRegister::Cache HardwareRegister::cv_cache {};

//------------------------------------------------------------------------------

bool HardwareRegister::Cache::query(const Chip & i_chip,
                                    const HardwareRegister * i_hwReg) const
{
    // Does i_chip exist in the cache?
    auto chipPairItr = iv_cache.find(i_chip);
    if (iv_cache.end() != chipPairItr)
    {
        auto & hwRegMap = chipPairItr->second; // for ease of use

        // Does i_hwReg exist in the cache?
        auto hwRegPairItr = hwRegMap.find(i_hwReg);
        if (hwRegMap.end() != hwRegPairItr)
        {
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------

BitString & HardwareRegister::Cache::access(const Chip & i_chip,
                                            const HardwareRegister * i_hwReg)
{
    // If the entry does not exist, create a new entry.
    if (!query(i_chip, i_hwReg))
    {
        BitString * bs = new BitStringBuffer { i_hwReg->getSize() * 8 };
        iv_cache[i_chip][i_hwReg] = bs;
    }

    // Return a reference to the target entry.
    return *(iv_cache[i_chip][i_hwReg]);
}

//------------------------------------------------------------------------------

void HardwareRegister::Cache::flush()
{
    // Delete all of the BitStrings.
    for (auto & chipPair : iv_cache)
    {
        for (auto & hwRegPair : chipPair.second)
        {
            delete hwRegPair.second;
        }
    }

    // !!! Do not delete the HardwareRegisters !!!
    // Those are deleted when the main uninitialize() API is called.

    // Flush the rest of the cache.
    iv_cache.clear();
}

//------------------------------------------------------------------------------

void HardwareRegister::Cache::flush(const Chip & i_chip,
                                    const HardwareRegister * i_hwReg)
{
    // Does i_chip exist in the cache?
    auto chipPairItr = iv_cache.find(i_chip);
    if (iv_cache.end() != chipPairItr)
    {
        auto & hwRegMap = chipPairItr->second; // for ease of use

        // Does i_hwReg exist in the cache?
        auto hwRegPairItr = hwRegMap.find(i_hwReg);
        if (hwRegMap.end() != hwRegPairItr)
        {
            delete hwRegPairItr->second; // delete the BitString
            hwRegMap.erase(i_hwReg);     // remove the entry for this register
        }

        // If i_hwReg was the only entry for i_chip, we can remove i_chip from
        // the cache.
        if (hwRegMap.empty())
        {
            iv_cache.erase(i_chip);
        }
    }
}

//------------------------------------------------------------------------------

} // end namespace libhei
