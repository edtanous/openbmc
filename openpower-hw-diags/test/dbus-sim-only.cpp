#include <util/dbus.hpp>

namespace util
{

namespace dbus
{

MachineType getMachineType()
{
    // default to Rainier 2S4U
    MachineType machineType = MachineType::Rainier_2S4U;

    return machineType;
}

} // namespace dbus

} // namespace util
