#include <libpdbg.h>

#include <attn/attn_handler.hpp>

// The attnHandler() function is called when the an attention GPIO event is
// triggered. We call it here directly to simulatea a GPIO event.
int main()
{
    int rc = 0; // return code

    // initialize pdbg targets
    pdbg_targets_init(nullptr);

    // exercise attention gpio event path
    attn::attnHandler(false);

    return rc;
}
