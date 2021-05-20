#include "nvmes.hpp"

#include <xyz/openbmc_project/Sensor/Threshold/Critical/server.hpp>
#include <xyz/openbmc_project/Sensor/Threshold/Warning/server.hpp>
#include <xyz/openbmc_project/Sensor/Value/server.hpp>

namespace phosphor
{
namespace nvme
{

void NvmeSSD::checkSensorThreshold()
{
    int8_t value = ValueIface::value();
    int8_t criticalHigh = CriticalInterface::criticalHigh();
    int8_t criticalLow = CriticalInterface::criticalLow();
    int8_t warningHigh = WarningInterface::warningHigh();
    int8_t warningLow = WarningInterface::warningLow();
    auto criticalAlarmLowState = CriticalInterface::criticalAlarmLow();
    auto criticalAlarmHighState = CriticalInterface::criticalAlarmHigh();
    auto warningAlarmLowState = WarningInterface::warningAlarmLow();
    auto warningAlarmHighState = WarningInterface::warningAlarmHigh();
    bool criticalAlarmLow = (value <= criticalLow);
    bool criticalAlarmHigh = (value >= criticalHigh);
    bool warningAlarmLow = (value <= warningLow);
    bool warningAlarmHigh = (value >= warningHigh);

    CriticalInterface::criticalAlarmHigh(criticalAlarmHigh);

    CriticalInterface::criticalAlarmLow(criticalAlarmLow);

    WarningInterface::warningAlarmHigh(warningAlarmHigh);

    WarningInterface::warningAlarmLow(warningAlarmLow);

    if (criticalAlarmHighState != criticalAlarmHigh)
    {
        if (criticalAlarmHigh)
        {
            CriticalInterface::criticalHighAlarmAsserted(value);
        }
        else
        {
            CriticalInterface::criticalHighAlarmDeasserted(value);
        }
    }
    if (criticalAlarmLowState != criticalAlarmLow)
    {
        if (criticalAlarmLow)
        {
            CriticalInterface::criticalLowAlarmAsserted(value);
        }
        else
        {
            CriticalInterface::criticalLowAlarmDeasserted(value);
        }
    }
    if (warningAlarmHighState != warningAlarmHigh)
    {
        if (warningAlarmHigh)
        {
            WarningInterface::warningHighAlarmAsserted(value);
        }
        else
        {
            WarningInterface::warningHighAlarmDeasserted(value);
        }
    }
    if (warningAlarmLowState != warningAlarmLow)
    {
        if (warningAlarmLow)
        {
            WarningInterface::warningLowAlarmAsserted(value);
        }
        else
        {
            WarningInterface::warningLowAlarmDeasserted(value);
        }
    }
}

void NvmeSSD::setSensorThreshold(int8_t criticalHigh, int8_t criticalLow,
                                 int8_t warningHigh, int8_t warningLow)
{
    CriticalInterface::criticalHigh(criticalHigh);
    CriticalInterface::criticalLow(criticalLow);

    WarningInterface::warningHigh(warningHigh);
    WarningInterface::warningLow(warningLow);
}

void NvmeSSD::setSensorMaxMin(int8_t maxValue, int8_t minValue)
{
    ValueIface::maxValue(maxValue);
    ValueIface::minValue(minValue);
}

void NvmeSSD::setSensorValueToDbus(const int8_t value)
{
    ValueIface::value(value);
}

} // namespace nvme
} // namespace phosphor
