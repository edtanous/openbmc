#pragma once

#include <sdbusplus/bus.hpp>
#include <sdbusplus/server/object.hpp>
#include <sdeventplus/clock.hpp>
#include <sdeventplus/event.hpp>
#include <sdeventplus/utility/timer.hpp>
#include <xyz/openbmc_project/Memory/MemoryECC/server.hpp>

namespace phosphor
{
namespace memory
{

template <typename... T>
using ServerObject = typename sdbusplus::server::object::object<T...>;

using EccInterface = sdbusplus::xyz::openbmc_project::Memory::server::MemoryECC;
using EccObject = ServerObject<EccInterface>;
/** @class
 *  @brief Manages ECC
 */
class ECC : sdbusplus::server::object::object<
                sdbusplus::xyz::openbmc_project::Memory::server::MemoryECC>
{
  public:
    ECC() = delete;
    ~ECC() = default;
    ECC(const ECC&) = delete;
    ECC& operator=(const ECC&) = delete;
    ECC(ECC&&) = default;
    ECC& operator=(ECC&&) = default;

    /** @brief Constructs
     *
     * @param[in] bus     - Handle to system dbus
     * @param[in] objPath - The Dbus path
     */
    ECC(sdbusplus::bus::bus& bus, const std::string& objPath) :
        sdbusplus::server::object::object<
            sdbusplus::xyz::openbmc_project::Memory::server::MemoryECC>(
            bus, objPath.c_str()),
        _bus(bus), _event(sdeventplus::Event::get_default()),
        _timer(_event, std::bind(&ECC::read, this)){
            // Nothing to do here
        };

    int64_t previousCeCounter = 0;
    int64_t previousUeCounter = 0;
    int64_t maxECCLog = 0;

    void run();
    void controlEDACReport(std::string);

  private:
    /** @brief sdbusplus bus client connection. */
    sdbusplus::bus::bus& _bus;
    /** @brief the Event Loop structure */
    sdeventplus::Event _event;
    /** @brief Read Timer */
    sdeventplus::utility::Timer<sdeventplus::ClockId::Monotonic> _timer;
    /** @brief Read sysfs entries */
    void read();

    /** @brief Set up D-Bus object init */
    void init();

    std::string getValue(std::string);

    void writeValue(std::string, std::string);
    // set ce_count to dbus
    int checkCeCount();
    // set ue_count to dbus
    int checkUeCount();
    // set eccErrorCount to dbus
    void checkEccLogFull(int64_t, int64_t);

    void resetCounter();
    // set maxECCLog value
    void getMaxLogValue();

    void addSELLog(std::string, std::string, std::vector<uint8_t>, bool,
                   uint16_t);
};

} // namespace memory
} // namespace phosphor
