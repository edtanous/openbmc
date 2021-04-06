#pragma once

#include <cstddef> // for size_t

namespace attn
{
namespace pel
{

enum class SectionID
{
    privateHeader  = 0x5048, // 'PH'
    userHeader     = 0x5548, // 'UH'
    primarySRC     = 0x5053, // 'PS'
    extendedHeader = 0x4548, // 'EH'
};

enum class ComponentID
{
    attentionHandler = 0xd100
};

enum class CreatorID
{
    hostboot   = 'B',
    hypervisor = 'H',
    openbmc    = 'O'
};

enum class SubsystemID
{
    hypervisor = 0x82,
    hostboot   = 0x8a,
    openbmc    = 0x8d
};

enum class Severity
{
    information = 0x00,
    termination = 0x51
};

enum class EventType
{
    na    = 0x00,
    trace = 0x02
};

enum class ActionFlags
{
    service = 0x8000,
    report  = 0x2000,
    call    = 0x0800
};

inline ActionFlags operator|(ActionFlags a, ActionFlags b)
{
    return static_cast<ActionFlags>(static_cast<int>(a) | static_cast<int>(b));
}

enum class EventScope
{
    platform = 0x03
};

constexpr size_t numSrcWords = 8;  // number of SRC hex words
const size_t asciiStringSize = 32; // size of SRC ascii string
const size_t mtmsSize        = 20; // size of an mtms field

} // namespace pel
} // namespace attn
