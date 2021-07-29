#include "config.h"

#include "extensions/phal/create_pel.hpp"
#include "extensions/phal/pdbg_utils.hpp"
#include "registration.hpp"

#include <fmt/format.h>
#include <sys/wait.h>
#include <unistd.h>

#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/elog.hpp>
#include <phosphor-logging/log.hpp>
#include <xyz/openbmc_project/Common/error.hpp>

#include <filesystem>

namespace openpower
{
namespace phal
{

using namespace phosphor::logging;

void importDevtree()
{
    namespace fs = std::filesystem;

    // check import data file is present
    auto path = fs::path(DEVTREE_EXP_FILE);
    if (!fs::exists(path))
    {
        // No import data file skip devtree import
        return;
    }

    // Update PDBG_DTB value
    openpower::phal::setDevtreeEnv();

    int status = 0;
    pid_t pid = fork();
    if (pid == 0)
    {
        std::string cmd("/usr/bin/attributes ");
        cmd += "import ";
        cmd += DEVTREE_EXP_FILE;
        execl("/bin/sh", "sh", "-c", cmd.c_str(), 0);

        auto error = errno;
        log<level::ERR>(fmt::format("Error occurred during attributes import "
                                    "execution, errno({})",
                                    error)
                            .c_str());
    }
    else if (pid > 0)
    {
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status))
        {
            log<level::ERR>("Failed to import attribute data");
            openpower::pel::createPEL("org.open_power.PHAL.Error.devtreeSync");
            return;
        }
    }
    else
    {
        log<level::ERR>("fork() failed.");
        throw std::runtime_error("importDevtree: fork() failed.");
    }

    try
    {
        // Delete attribute data file once updated.
        if (fs::exists(path))
        {
            // delete export data file
            fs::remove_all(path);
        }
    }
    catch (fs::filesystem_error& e)
    { // Log message and continue. Data already applied successfully.
        log<level::ERR>(fmt::format("File({}) delete failed Error:({})",
                                    DEVTREE_EXP_FILE, e.what())
                            .c_str());
    }

    log<level::INFO>("Successfully imported devtree attribute data");
}

REGISTER_PROCEDURE("importDevtree", importDevtree)

} // namespace phal
} // namespace openpower
