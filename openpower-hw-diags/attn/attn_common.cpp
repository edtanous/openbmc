#include <libpdbg.h>

#include <attn/attn_common.hpp>
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

} // namespace attn
