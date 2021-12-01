#include <libpdbg.h>

#include <attn/attn_common.hpp>
#include <attn/attn_handler.hpp>
#include <attn/attn_logging.hpp>
#include <sdbusplus/bus.hpp>
#include <util/pdbg.hpp>

#include <iomanip>
#include <iostream>
#include <map>

namespace attn
{

/** @brief Traces some regs for hostboot */
void addHbStatusRegs()
{
    // Only do this for P10 systems
    if (util::pdbg::queryHardwareAnalysisSupported())
    {
        // We only need this for PRIMARY processor
        pdbg_target* pibTarget = pdbg_target_from_path(nullptr, "/proc0/pib");
        pdbg_target* fsiTarget = pdbg_target_from_path(nullptr, "/proc0/fsi");

        uint32_t l_cfamData  = 0xFFFFFFFF;
        uint64_t l_scomData1 = 0xFFFFFFFFFFFFFFFFull;
        uint64_t l_scomData2 = 0xFFFFFFFFFFFFFFFFull;
        uint32_t l_cfamAddr  = 0x283C;
        uint64_t l_scomAddr1 = 0x4602F489;
        uint64_t l_scomAddr2 = 0x4602F487;

        if ((nullptr != fsiTarget) && (nullptr != pibTarget))
        {
            // buffer for formatted strings (+1 for null, just in case)
            char buffer[sizeof("some read error: 0x0123456789ABCDEF ")];

            // get first debug reg (CFAM)
            if (RC_SUCCESS != fsi_read(fsiTarget, l_cfamAddr, &l_cfamData))
            {
                sprintf(buffer, "cfam read error: 0x%08x", l_cfamAddr);
                trace<level::ERROR>(buffer);
                l_cfamData = 0xFFFFFFFF;
            }

            // Get SCOM regs next (just 2 of them)
            if (RC_SUCCESS != pib_read(pibTarget, l_scomAddr1, &l_scomData1))
            {
                sprintf(buffer, "scom read error: 0x%016" PRIx64 "",
                        l_scomAddr1);
                trace<level::ERROR>(buffer);
                l_scomData1 = 0xFFFFFFFFFFFFFFFFull;
            }

            if (RC_SUCCESS != pib_read(pibTarget, l_scomAddr2, &l_scomData2))
            {
                sprintf(buffer, "scom read error: 0x%016" PRIx64 "",
                        l_scomAddr2);
                trace<level::ERROR>(buffer);
                l_scomData2 = 0xFFFFFFFFFFFFFFFFull;
            }
        }

        // Trace out the results here of all 3 regs
        // (Format should resemble FSP: HostBoot Reg:0000283C  Data:AA801504
        // 00000000  Proc:00050001 )
        std::stringstream ss1, ss2, ss3;

        ss1 << "HostBoot Reg:" << std::setw(8) << std::setfill('0') << std::hex
            << l_cfamAddr << " Data:" << l_cfamData << " Proc:00000000";

        ss2 << "HostBoot Reg:" << std::setw(8) << std::setfill('0') << std::hex
            << l_scomAddr1 << " Data:" << std::setw(16) << l_scomData1
            << " Proc:00000000";

        ss3 << "HostBoot Reg:" << std::setw(8) << std::setfill('0') << std::hex
            << l_scomAddr2 << " Data:" << std::setw(16) << l_scomData2
            << " Proc:00000000";

        std::string strobj1 = ss1.str();
        std::string strobj2 = ss2.str();
        std::string strobj3 = ss3.str();

        trace<level::INFO>(strobj1.c_str());
        trace<level::INFO>(strobj2.c_str());
        trace<level::INFO>(strobj3.c_str());
    }

    return;

} // end addHbStatusRegs

/** @brief Check for recoverable errors present */
bool recoverableErrors()
{
    bool recoverableErrors = false; // assume no recoverable attentions

    pdbg_target* target;
    pdbg_for_each_class_target("proc", target)
    {
        if (PDBG_TARGET_ENABLED == pdbg_target_probe(target))
        {
            auto proc = pdbg_target_index(target); // get processor number

            // Use PIB target to determine if a processor is enabled
            char path[16];
            sprintf(path, "/proc%d/pib", proc);
            pdbg_target* pibTarget = pdbg_target_from_path(nullptr, path);

            // sanity check
            if (nullptr == pibTarget)
            {
                trace<level::INFO>("pib path or target not found");
                continue;
            }

            // check if pib target is enabled - indicates proc is enabled
            if (PDBG_TARGET_ENABLED == pdbg_target_probe(pibTarget))
            {
                // The processor FSI target is required for CFAM read
                sprintf(path, "/proc%d/fsi", proc);
                pdbg_target* fsiTarget = pdbg_target_from_path(nullptr, path);

                // sanity check
                if (nullptr == fsiTarget)
                {
                    trace<level::INFO>("fsi path or target not found");
                    continue;
                }

                uint32_t isr_val = 0xffffffff; // invalid isr value

                // get active attentions on processor
                if (RC_SUCCESS != fsi_read(fsiTarget, 0x1007, &isr_val))
                {
                    // log cfam read error
                    trace<level::ERROR>("Error! cfam read 0x1007 FAILED");
                    eventAttentionFail((int)AttnSection::attnHandler |
                                       ATTN_PDBG_CFAM);
                }
                // check for invalid/stale value
                else if (0xffffffff == isr_val)
                {
                    trace<level::ERROR>("Error! cfam read 0x1007 INVALID");
                    continue;
                }
                // check recoverable error status bit
                else if (0 != (isr_val & RECOVERABLE_ATTN))
                {
                    recoverableErrors = true;
                    break;
                }
            } // fsi target enabled
        }     // pib target enabled
    }         // next processor

    return recoverableErrors;
}

/** @brief timesec less-than-equal-to compare */
bool operator<=(const timespec& lhs, const timespec& rhs)
{
    if (lhs.tv_sec == rhs.tv_sec)
        return lhs.tv_nsec <= rhs.tv_nsec;
    else
        return lhs.tv_sec <= rhs.tv_sec;
}

/** @brief sleep for n-seconds */
void sleepSeconds(const unsigned int seconds)
{
    auto count = seconds;
    struct timespec requested, remaining;

    while (0 < count)
    {
        requested.tv_sec  = 1;
        requested.tv_nsec = 0;
        remaining         = requested;

        while (-1 == nanosleep(&requested, &remaining))
        {
            // if not changing or implausible then abort
            if (requested <= remaining)
            {
                break;
            }

            // back to sleep
            requested = remaining;
        }
        count--;
    }
}

} // namespace attn
