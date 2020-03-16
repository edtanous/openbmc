#pragma once

#include <ipmid/api.h>
#include <stdint.h>

#include <map>
#include <string>

// IPMI commands for net functions.
enum ipmi_netfn_oem_cmds
{
    IPMI_CMD_DETECT_RISERF = 0x01,
};

constexpr int initError = -1;        /* The parameter of definition error */
constexpr unsigned char command = 0; /* The parameter of smbus command */
constexpr unsigned char a[] = {
    0x00, 0x00}; /* The response data : CPU0 & CPU1 are not present */
constexpr unsigned char b[] = {
    0x00, 0x01}; /* The response data : CPU1 is present but CPU0 isn't */
constexpr unsigned char c[] = {
    0x01, 0x00}; /* The response data : CPU0 is present but CPU1 isn't */
constexpr unsigned char d[] = {
    0x01, 0x01}; /* The response data : CPU0 & CPU1 are present */

ipmi_ret_t ipmi_wistron_detect_riserf(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                      ipmi_request_t request,
                                      ipmi_response_t response,
                                      ipmi_data_len_t data_len,
                                      ipmi_context_t context);
