#include "phalerror/phal_error.hpp"

#include <libipl.H>

#include <phosphor-logging/log.hpp>
#include <registration.hpp>
namespace openpower
{
namespace phal
{

using namespace phosphor::logging;

/**
 * @brief Starts the self boot engine on POWER processor position 0
 *        to kick off a boot.
 * @return void
 */
void startHost()
{
    // add callback methods for debug traces and for boot failures
    openpower::pel::addBootErrorCallbacks();

    // callback method will be called upon failure which will create the PEL
    if (ipl_init(IPL_DEFAULT) != 0)
    {
        log<level::ERR>("ipl_init failed");
        throw std::runtime_error("Boot initialization failed");
    }

    // callback method will be called upon failure which will create the PEL
    int rc = ipl_run_major(0);
    if (rc > 0)
    {
        log<level::ERR>("step 0 failed to start the host");
        throw std::runtime_error("Failed to execute host start boot step");
    }
}

REGISTER_PROCEDURE("startHost", startHost);

} // namespace phal
} // namespace openpower
