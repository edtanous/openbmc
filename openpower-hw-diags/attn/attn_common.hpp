#pragma once

namespace attn
{

enum class HostState
{
    Quiesce,
    Diagnostic
};

/**
 * @brief Transition the host state
 *
 * We will transition the host state by starting the appropriate dbus target.
 *
 * @param i_hostState the state to transition the host to
 */
void transitionHost(const HostState i_hostState);

} // namespace attn
