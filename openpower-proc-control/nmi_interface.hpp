#pragma once

#include <sdbusplus/bus.hpp>
#include <sdbusplus/server/object.hpp>
#include <xyz/openbmc_project/Control/Host/NMI/server.hpp>

namespace openpower
{
namespace proc
{

using Base = sdbusplus::xyz::openbmc_project::Control::Host::server::NMI;
using Interface = sdbusplus::server::object::object<Base>;

/*  @class NMI
 *  @brief Implementation of NMI (Soft Reset)
 */
class NMI : public Interface
{
  public:
    NMI() = delete;
    NMI(const NMI&) = delete;
    NMI& operator=(const NMI&) = delete;
    NMI(NMI&&) = delete;
    NMI& operator=(NMI&&) = delete;
    virtual ~NMI() = default;

    /*  @brief Constructor to put object onto bus at a dbus path.
     *  @param[in] bus - sdbusplus D-Bus to attach to.
     *  @param[in] path - Path to attach to.
     */
    NMI(sdbusplus::bus::bus& bus, const char* path);

    /*  @brief trigger stop followed by soft reset.
     */
#ifdef SDBUSPP_NEW_CAMELCASE
    void nmi() override;
#else
    void nMI() override;
#endif

  private:
    /** @brief sdbus handle */
    sdbusplus::bus::bus& bus;

    /** @brief object path */
    std::string objectPath;
};

} // namespace proc
} // namespace openpower
