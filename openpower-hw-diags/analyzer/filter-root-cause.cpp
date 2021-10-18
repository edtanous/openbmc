#include <assert.h>

#include <hei_main.hpp>

#include <algorithm>
#include <limits>
#include <string>

namespace analyzer
{

//------------------------------------------------------------------------------

bool filterRootCause(const libhei::IsolationData& i_isoData,
                     libhei::Signature& o_rootCause)
{
    // We'll need to make a copy of the list so that the original list is
    // maintained for the log.
    std::vector<libhei::Signature> list{i_isoData.getSignatureList()};

    // START WORKAROUND
    // TODO: Filtering should be data driven. Until that support is available,
    //       use the following isolation rules.

    // Special and host attentions are not supported by this user application.
    auto itr = std::remove_if(list.begin(), list.end(), [&](const auto& t) {
        return (libhei::ATTN_TYPE_SP_ATTN == t.getAttnType() ||
                libhei::ATTN_TYPE_HOST_ATTN == t.getAttnType());
    });
    list.resize(std::distance(list.begin(), itr));

    // TODO: This is a rudimentary filter that first looks for any recoverable
    //       attention, then any unit checkstop, and then any system checkstop.
    //       This is built on the premise that recoverable errors could be the
    //       root cause of an system checkstop attentions. Fortunately, we
    //       just need to sort the list by the greater attention type value.
    std::sort(list.begin(), list.end(), [&](const auto& a, const auto& b) {
        return a.getAttnType() > b.getAttnType();
    });
    if (!list.empty())
    {
        // The entry at the front of the list will be the root cause.
        o_rootCause = list.front();
        return true;
    }

    // END WORKAROUND

    return false; // default, no active attentions found.
}

//------------------------------------------------------------------------------

} // namespace analyzer
