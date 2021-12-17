//------------------------------------------------------------------------------
// IMPORTANT:
// This file will ONLY be built in CI test and should be used for any functions
// that require addition support to simulate in CI test. Any functions that will
// work out-of-the-box in CI test with use of the fake device tree should be put
// in `pdbg.cpp`.
//------------------------------------------------------------------------------

#include <assert.h>

#include <util/pdbg.hpp>
#include <util/trace.hpp>

//------------------------------------------------------------------------------

// Using this to fake the value returned from the simulation-only version of
// util::pdbg::queryLpcTimeout().
bool g_lpcTimeout = false;

namespace util
{
namespace pdbg
{

// This is the simulated version of this function.
bool queryLpcTimeout(pdbg_target* target)
{
    // Must be a processor target.
    assert(TYPE_PROC == getTrgtType(target));

    // Instead of the SBE chip-op, use the faked value.
    return g_lpcTimeout;
}

} // namespace pdbg
} // namespace util
