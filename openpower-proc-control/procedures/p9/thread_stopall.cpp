#include "registration.hpp"

#include <fmt/format.h>
extern "C"
{
#include <libpdbg.h>
}
#include <phosphor-logging/log.hpp>

namespace openpower
{
namespace phal
{
using namespace phosphor::logging;

/**
 * @brief Stop instruction executions on all functional threads in the
 *        host processors.
 *        This procedure is used to stop all threads in the system in
 *        attempt best case approach. Like issue processor level stopall
 *        chip-op with ignore hardware error mode. Since this function
 *        is used in power-off/error path, ignore the internal error now.
 */
void threadStopAll(void)
{
    // Set pdbg back-end to sbefifo.
    pdbg_set_backend(PDBG_BACKEND_SBEFIFO, NULL);

    // initialize the pdbg.
    pdbg_targets_init(NULL);

    struct pdbg_target* pibTarget;

    pdbg_for_each_class_target("pib", pibTarget)
    {
        // probe pib traget.
        pdbg_target_probe(pibTarget);
    }

    // Issue system level thread stop
    if (thread_stop_all() < 0)
    {
        log<level::ERR>("Failed to stop all threads");
        return;
    }
    log<level::INFO>("Processor thread stopall completed");
}

REGISTER_PROCEDURE("threadStopAll", threadStopAll)

} // namespace phal
} // namespace openpower
