#include "phalerror/phal_error.hpp"
#include "procedures/phal/common_utils.hpp"
#include "registration.hpp"

#include <libekb.H>

#include <phosphor-logging/log.hpp>

namespace openpower
{
namespace phal
{

using namespace phosphor::logging;

void prePoweroff(void)
{
    try
    {
        phal_init();
    }
    catch (const std::exception& ex)
    {
        log<level::ERR>("Exception raised during init PHAL",
                        entry("EXCEPTION=%s", ex.what()));
        openpower::pel::detail::processBootErrorCallback(false);
        // Dont throw exception on failure because, we need to proceed
        // further eventhough there is failure for proc-pre-poweroff
        return;
    }

    // To clear trace if success
    openpower::pel::detail::processBootErrorCallback(true);

    // callback method will be called upon failure which will create the PEL
    int rc = ipl_pre_poweroff();
    if (rc)
    {
        log<level::ERR>("pre_poweroff failed");
        // Dont throw exception on failure because, we need to proceed
        // further eventhough there is failure for proc-pre-poweroff
        return;
    }
}

REGISTER_PROCEDURE("prePoweroff", prePoweroff)

} // namespace phal
} // namespace openpower
