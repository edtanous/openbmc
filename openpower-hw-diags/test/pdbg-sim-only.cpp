//------------------------------------------------------------------------------
// IMPORTANT:
// This file will ONLY be built in CI test and should be used for any functions
// that require addition support to simulate in CI test. Any functions that will
// work out-of-the-box in CI test with use of the fake device tree should be put
// in `pdbg.cpp`.
//------------------------------------------------------------------------------

#include <assert.h>

#include <test/sim-hw-access.hpp>
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

//------------------------------------------------------------------------------

// This is the simulated version of this function.
bool queryLpcTimeout(pdbg_target* target)
{
    // Must be a processor target.
    assert(TYPE_PROC == getTrgtType(target));

    // Instead of the SBE chip-op, use the faked value.
    return g_lpcTimeout;
}

//------------------------------------------------------------------------------

int getScom(pdbg_target* i_target, uint64_t i_addr, uint64_t& o_val)
{
    assert(nullptr != i_target);
    assert(TYPE_PROC == getTrgtType(i_target) ||
           TYPE_OCMB == getTrgtType(i_target));

    int rc = sim::ScomAccess::getSingleton().get(i_target, i_addr, o_val);

    if (0 != rc)
    {
        trace::err("SCOM read failure: target=%s addr=0x%0" PRIx64,
                   getPath(i_target), i_addr);
    }

    return rc;
}

//------------------------------------------------------------------------------

int getCfam(pdbg_target* i_target, uint32_t i_addr, uint32_t& o_val)
{
    assert(nullptr != i_target);
    assert(TYPE_PROC == getTrgtType(i_target));

    int rc = sim::CfamAccess::getSingleton().get(i_target, i_addr, o_val);

    if (0 != rc)
    {
        trace::err("CFAM read failure: target=%s addr=0x%08x",
                   getPath(i_target), i_addr);
    }

    return rc;
}

//------------------------------------------------------------------------------

} // namespace pdbg
} // namespace util
