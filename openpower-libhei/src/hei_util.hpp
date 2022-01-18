#pragma once

#include <assert.h>

#include <hei_types.hpp>

#include <limits>
#include <string>

namespace libhei
{

//------------------------------------------------------------------------------

/**
 * @brief The string hashing algorithm used for register and isolation node
 *        names.
 */
inline uint64_t hash(unsigned int i_bytes, const std::string& i_str)
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

template <class T>
inline T hash(const std::string& i_str)
{
    return static_cast<T>(hash(sizeof(T), i_str));
}

// Specialization for RegisterId_t because they are only a 3-byte field.
template <>
inline RegisterId_t hash<RegisterId_t>(const std::string& i_str)
{
    return static_cast<RegisterId_t>(hash(3, i_str));
}

//------------------------------------------------------------------------------

} // namespace libhei
