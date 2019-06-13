#include "config.h"

#include <sdbusplus/bus.hpp>
#include <sdbusplus/server.hpp>
#include <sdbusplus/server/object.hpp>
#include <sdeventplus/clock.hpp>
#include <sdeventplus/event.hpp>
#include <sdeventplus/utility/timer.hpp>

namespace phosphor
{
namespace nvme
{

class Nvme
{
  public:
    Nvme() = delete;
    Nvme(const Nvme&) = delete;
    Nvme& operator=(const Nvme&) = delete;
    Nvme(Nvme&&) = delete;
    Nvme& operator=(Nvme&&) = delete;

    Nvme(sdbusplus::bus::bus& bus) :
        bus(bus), _event(sdeventplus::Event::get_default()),
        _timer(_event, std::bind(&Nvme::read, this))
    {
    }

    void run();

  private:
    sdbusplus::bus::bus& bus;

    sdeventplus::Event _event;
    /** @brief Read Timer */
    sdeventplus::utility::Timer<sdeventplus::ClockId::Monotonic> _timer;

    void init();
    void read();
};
} // namespace nvme
} // namespace phosphor