#include <libpdbg.h>

#include <attn/attn_config.hpp>
#include <attn/attn_handler.hpp>
#include <cli.hpp>

/** @brief Attention handler test application */
int main(int argc, char* argv[])
{
    int rc = 0; // return code

    // initialize pdbg targets
    pdbg_targets_init(nullptr);

    // create attention handler config object
    attn::Config attnConfig;

    // convert cmd line args to config values
    parseConfig(argv, argv + argc, &attnConfig);

    // exercise attention gpio event path
    attn::attnHandler(&attnConfig);

    return rc;
}
