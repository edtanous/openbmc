#pragma once

#include <host-ipmid/ipmid-api.h>

namespace local
{
namespace users
{

/**
 * @brief Enable all local BMC users
 *
 * Sets the UserEnabled property on all
 * /xyz/openbmc_project/user/<user> objects.
 *
 * @return ipmi_ret_t - IPMI CC
 */
ipmi_ret_t enableUsers();

}
}
