#pragma once
#include <cstdint>

namespace attn
{

/** @brief Dump types supported by dump request */
enum class DumpType
{
    Hostboot,
    Hardware,
    SBE
};

/** @brief Structure for dump request parameters */
class DumpParameters
{
  public:
    uint32_t logId;
    uint32_t unitId;
    DumpType dumpType;
};

/**
 * Request a dump from the dump manager
 *
 * Request a dump from the dump manager and register a monitor for observing
 * the dump progress.
 *
 * @param dumpParameters Parameters for the dump request
 */
void requestDump(const DumpParameters& dumpParameters);

} // namespace attn
