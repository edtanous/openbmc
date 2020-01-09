#include <libpdbg.h>

#include <attn_handler.hpp>

/**
 * @brief Attention handler application main()
 *
 * This is the main interface to the Attention handler application, it will
 * initialize the libgpd targets and start a gpio mointor.
 *
 * @return 0 = success
 */
int main()
{
    int rc = 0; // return code

    // initialize pdbg targets
    pdbg_targets_init(nullptr);

    return rc;
}
