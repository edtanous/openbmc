//------------------------------------------------------------------------------
// IMPORTANT:
// This file will NOT be built in CI test and should be used for any functions
// that require addition support to simulate in CI test. Any functions that will
// work out-of-the-box in CI test with use of the fake device tree should be put
// in `pdbg.cpp`.
//------------------------------------------------------------------------------

#include <assert.h>

extern "C"
{
#include <libpdbg_sbe.h>
}

#include <util/pdbg.hpp>
#include <util/trace.hpp>

using namespace analyzer;

namespace util
{

namespace pdbg
{

//------------------------------------------------------------------------------

bool queryLpcTimeout(pdbg_target* target)
{
    // Must be a processor target.
    assert(TYPE_PROC == getTrgtType(target));

    uint32_t result = 0;
    if (0 != sbe_lpc_timeout(util::pdbg::getPibTrgt(target), &result))
    {
        trace::err("sbe_lpc_timeout() failed: target=%s", getPath(target));
        result = 0; // just in case
    }

    // 0 if no timeout, 1 if LPC timeout occurred.
    return (0 != result);
}

//------------------------------------------------------------------------------

} // namespace pdbg

} // namespace util
