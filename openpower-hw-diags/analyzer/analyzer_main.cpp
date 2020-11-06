#include <assert.h>
#include <libpdbg.h>
#include <unistd.h>

#include <hei_main.hpp>
#include <phosphor-logging/log.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace analyzer
{

//------------------------------------------------------------------------------

// Forward references for externally defined functions.

void initializeIsolator(const std::vector<libhei::Chip>& i_chips);

//------------------------------------------------------------------------------

uint8_t __attrType(pdbg_target* i_trgt)
{
    uint8_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_TYPE", 1, 1, &attr);
    return attr;
}

uint32_t __attrFapiPos(pdbg_target* i_trgt)
{
    uint32_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_FAPI_POS", 4, 1, &attr);
    return attr;
}

//------------------------------------------------------------------------------

const char* __attn(libhei::AttentionType_t i_attnType)
{
    const char* str = "";
    switch (i_attnType)
    {
        case libhei::ATTN_TYPE_CHECKSTOP:
            str = "CHECKSTOP";
            break;
        case libhei::ATTN_TYPE_UNIT_CS:
            str = "UNIT_CS";
            break;
        case libhei::ATTN_TYPE_RECOVERABLE:
            str = "RECOVERABLE";
            break;
        case libhei::ATTN_TYPE_SP_ATTN:
            str = "SP_ATTN";
            break;
        case libhei::ATTN_TYPE_HOST_ATTN:
            str = "HOST_ATTN";
            break;
        default:
            trace::err("Unsupported attention type: %u", i_attnType);
            assert(0);
    }
    return str;
}

uint32_t __trgt(const libhei::Signature& i_sig)
{
    auto trgt = (pdbg_target*)i_sig.getChip().getChip();

    uint8_t type = __attrType(trgt);
    uint32_t pos = __attrFapiPos(trgt);

    // Technically, the FapiPos attribute is 32-bit, but not likely to ever go
    // over 24-bit.

    return type << 24 | (pos & 0xffffff);
}

uint32_t __sig(const libhei::Signature& i_sig)
{
    return i_sig.getId() << 16 | i_sig.getInstance() << 8 | i_sig.getBit();
}

//------------------------------------------------------------------------------

// Returns the chip model/level of the given target.
libhei::ChipType_t __getChipType(pdbg_target* i_trgt)
{
    libhei::ChipType_t type;

    // START WORKAROUND
    // TODO: Will need to grab the model/level from the target attributes when
    //       they are available. For now, use ATTR_TYPE to determine which
    //       currently supported value to use supported.
    uint8_t attrType = __attrType(i_trgt);
    switch (attrType)
    {
        case 0x05: // PROC
            type = 0x120DA049;
            break;

        case 0x4b: // OCMB_CHIP
            type = 0x160D2000;
            break;

        default:
            trace::err("Unsupported ATTR_TYPE value: 0x%02x", attrType);
            assert(0);
    }
    // END WORKAROUND

    return type;
}

//------------------------------------------------------------------------------

// Gathers list of active chips to analyze.
void __getActiveChips(std::vector<libhei::Chip>& o_chips)
{
    // Iterate each processor.
    pdbg_target* procTrgt;
    pdbg_for_each_class_target("proc", procTrgt)
    {
        // Active processors only.
        if (PDBG_TARGET_ENABLED != pdbg_target_probe(procTrgt))
            continue;

        // Add the processor to the list.
        o_chips.emplace_back(procTrgt, __getChipType(procTrgt));

        // Iterate the connected OCMBs, if they exist.
        pdbg_target* ocmbTrgt;
        pdbg_for_each_target("ocmb", procTrgt, ocmbTrgt)
        {
            // Active OCMBs only.
            if (PDBG_TARGET_ENABLED != pdbg_target_probe(ocmbTrgt))
                continue;

            // Add the OCMB to the list.
            o_chips.emplace_back(ocmbTrgt, __getChipType(ocmbTrgt));
        }
    }
}

//------------------------------------------------------------------------------

// Takes a signature list that will be filtered and sorted. The first entry in
// the returned list will be the root cause. If the returned list is empty,
// analysis failed.
void __filterRootCause(std::vector<libhei::Signature>& io_list)
{
    // For debug, trace out the original list of signatures before filtering.
    for (const auto& sig : io_list)
    {
        trace::inf("Signature: %s 0x%0" PRIx32 " %s",
                   util::pdbg::getPath(sig.getChip()), __sig(sig),
                   __attn(sig.getAttnType()));
    }

    // Special and host attentions are not supported by this user application.
    auto newEndItr =
        std::remove_if(io_list.begin(), io_list.end(), [&](const auto& t) {
            return (libhei::ATTN_TYPE_SP_ATTN == t.getAttnType() ||
                    libhei::ATTN_TYPE_HOST_ATTN == t.getAttnType());
        });

    // Shrink the vector, if needed.
    io_list.resize(std::distance(io_list.begin(), newEndItr));

    // START WORKAROUND
    // TODO: Filtering should be determined by the RAS Data Files provided by
    //       the host firmware via the PNOR (similar to the Chip Data Files).
    //       Until that support is available, use a rudimentary filter that
    //       first looks for any recoverable attention, then any unit checkstop,
    //       and then any system checkstop. This is built on the premise that
    //       recoverable errors could be the root cause of an system checkstop
    //       attentions. Fortunately, we just need to sort the list by the
    //       greater attention type value.
    std::sort(io_list.begin(), io_list.end(),
              [&](const auto& a, const auto& b) {
                  return a.getAttnType() > b.getAttnType();
              });
    // END WORKAROUND
}

//------------------------------------------------------------------------------

bool __logError(const std::vector<libhei::Signature>& i_sigList,
                const libhei::IsolationData& i_isoData)
{
    bool attnFound = false;

    // Get numerical values for the root cause.
    uint32_t word6 = 0; // [ 0: 7]: chip target type
                        // [ 8:31]: chip FAPI position
                        //    uint32_t word7 = 0; // TODO: chip target info
    uint32_t word8 = 0; // [ 0:15]: node ID
                        // [16:23]: node instance
                        // [24:31]: bit position
                        //    uint32_t word9 = 0; // [ 0: 7]: attention type

    if (i_sigList.empty())
    {
        trace::inf("No active attentions found");
    }
    else
    {
        attnFound = true;

        // The root cause attention is the first in the filtered list.
        libhei::Signature root = i_sigList.front();

        word6 = __trgt(root);
        word8 = __sig(root);

        trace::inf("Root cause attention: %s 0x%0" PRIx32 " %s",
                   util::pdbg::getPath(root.getChip()), word8,
                   __attn(root.getAttnType()));
    }

    // Get the log data.
    std::map<std::string, std::string> logData;
    logData["_PID"]      = std::to_string(getpid());
    logData["CHIP_ID"]   = std::to_string(word6);
    logData["SIGNATURE"] = std::to_string(word8);

    // Get access to logging interface and method for creating log.
    auto bus = sdbusplus::bus::new_default_system();

    // Using direct create method (for additional data)
    auto method = bus.new_method_call(
        "xyz.openbmc_project.Logging", "/xyz/openbmc_project/logging",
        "xyz.openbmc_project.Logging.Create", "Create");

    // Attach additional data
    method.append("org.open_power.HwDiags.Error.Checkstop",
                  "xyz.openbmc_project.Logging.Entry.Level.Error", logData);

    // Log the event.
    // TODO: Should the reply be handled?
    bus.call(method);

    return attnFound;
}

//------------------------------------------------------------------------------

bool analyzeHardware()
{
    bool attnFound = false;

    trace::inf(">>> enter analyzeHardware()");

    // Get the active chips to be analyzed.
    std::vector<libhei::Chip> chips;
    __getActiveChips(chips);

    // Initialize the isolator for all chips.
    trace::inf("Initializing the isolator...");
    initializeIsolator(chips);

    // Isolate attentions.
    trace::inf("Isolating errors: # of chips=%u", chips.size());
    libhei::IsolationData isoData{};
    libhei::isolate(chips, isoData);

    // Filter signatures to determine root cause. We'll need to make a copy of
    // the list so that the original list is maintained for the log.
    std::vector<libhei::Signature> sigList{isoData.getSignatureList()};
    __filterRootCause(sigList);

    // Create and commit a log.
    attnFound = __logError(sigList, isoData);

    // All done, clean up the isolator.
    trace::inf("Uninitializing isolator...");
    libhei::uninitialize();

    trace::inf("<<< exit analyzeHardware()");

    return attnFound;
}

} // namespace analyzer
