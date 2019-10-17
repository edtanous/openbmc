extern "C" {
#include <libipl.h>
}

#include "xyz/openbmc_project/Common/error.hpp"

#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <registration.hpp>
namespace openpower
{
namespace phal
{

using namespace phosphor::logging;
using namespace sdbusplus::xyz::openbmc_project::Common::Error;

/**
 * @brief Starts the self boot engine on POWER processor position 0
 *        to kick off a boot.
 * @return void
 */
void startHost()
{
    if (ipl_init() != 0)
    {
        log<level::ERR>("ipl_init failed");
        // TODO ibm-openbmc#1470
        elog<InternalFailure>();
    }

    if (ipl_run_major(0) > 0)
    {
        log<level::ERR>("step 0 failed to start the host");
        // TODO ibm-openbmc#1470
        elog<InternalFailure>();
    }
}

REGISTER_PROCEDURE("startHost", startHost);

} // namespace phal
} // namespace openpower
