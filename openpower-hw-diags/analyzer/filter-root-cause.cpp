#include <assert.h>

#include <hei_main.hpp>

#include <algorithm>
#include <limits>
#include <string>

namespace analyzer
{

//------------------------------------------------------------------------------

uint64_t __hash(unsigned int i_bytes, const std::string& i_str)
{
    // This hash is a simple "n*s[0] + (n-1)*s[1] + ... + s[n-1]" algorithm,
    // where s[i] is a chunk from the input string the length of i_bytes.

    // Currently only supporting 1-8 byte hashes.
    assert(1 <= i_bytes && i_bytes <= sizeof(uint64_t));

    // Start hashing each chunk.
    uint64_t sumA = 0;
    uint64_t sumB = 0;

    // Iterate one chunk at a time.
    for (unsigned int i = 0; i < i_str.size(); i += i_bytes)
    {
        // Combine each chunk into a single integer value. If we reach the end
        // of the string, pad with null characters.
        uint64_t chunk = 0;
        for (unsigned int j = 0; j < i_bytes; j++)
        {
            chunk <<= 8;
            chunk |= (i + j < i_str.size()) ? i_str[i + j] : '\0';
        }

        // Apply the simple hash.
        sumA += chunk;
        sumB += sumA;
    }

    // Mask off everything except the target number of bytes.
    auto mask = std::numeric_limits<uint64_t>::max();
    sumB &= mask >> ((sizeof(uint64_t) - i_bytes) * 8);

    return sumB;
}

//------------------------------------------------------------------------------

bool __findRcsOscError(const std::vector<libhei::Signature>& i_list,
                       libhei::Signature& o_rootCause)
{
    // TODO: Consider returning all of them instead of one as root cause.
    auto itr = std::find_if(i_list.begin(), i_list.end(), [&](const auto& t) {
        return (__hash(2, "TP_LOCAL_FIR") == t.getId() &&
                (42 == t.getBit() || 43 == t.getBit()));
    });

    if (i_list.end() != itr)
    {
        o_rootCause = *itr;
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------

bool __findPllUnlock(const std::vector<libhei::Signature>& i_list,
                     libhei::Signature& o_rootCause)
{
    // TODO: Consider returning all of them instead of one as root cause.
    auto itr = std::find_if(i_list.begin(), i_list.end(), [&](const auto& t) {
        return (__hash(2, "PLL_UNLOCK") == t.getId() &&
                (0 == t.getBit() || 1 == t.getBit()));
    });

    if (i_list.end() != itr)
    {
        o_rootCause = *itr;
        return true;
    }

    return false;
}

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

    // Look for:
    //   - any RCS OSC errors (always first)
    //   - any PLL unlock attentions (always second)
    if (__findRcsOscError(list, o_rootCause) ||
        __findPllUnlock(list, o_rootCause))
    {
        return true;
    }

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
