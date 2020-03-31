#include <libpdbg.h>

#include <attn/attn_config.hpp>
#include <attn/attn_handler.hpp>
#include <cli.hpp>

/** @brief Attention handler test application */
int main(int argc, char* argv[])
{
    int rc = 0; // return code

    // attention handler configuration flags
    bool vital_enable     = true;
    bool checkstop_enable = true;
    bool ti_enable        = true;
    bool bp_enable        = true;

    // initialize pdbg targets
    pdbg_targets_init(nullptr);

    // convert cmd line args to config values
    parseConfig(argv, argv + argc, vital_enable, checkstop_enable, ti_enable,
                bp_enable);

    // create attention handler config object
    attn::Config config(vital_enable, checkstop_enable, ti_enable, bp_enable);

    // exercise attention gpio event path
    attn::attnHandler(&config);

    return rc;
}
