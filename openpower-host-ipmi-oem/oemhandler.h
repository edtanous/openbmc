#ifndef __HOST_IPMI_OPENPOWEROEM_HANDLER_H__
#define __HOST_IPMI_OPENPOWEROEM_HANDLER_H__

#include <stdint.h>
#include <host-ipmid/ipmid-api.h>


// IPMI commands for net functions.
enum ipmi_netfn_oem_cmds
{
    IPMI_CMD_PESEL = 0xF0,
};



ipmi_ret_t ipmi_ibm_oem_partial_esel(ipmi_netfn_t netfn, ipmi_cmd_t cmd, 
                              ipmi_request_t request, ipmi_response_t response, 
                              ipmi_data_len_t data_len, ipmi_context_t context);


struct esel_request_t {
    uint8_t  residls;
    uint8_t  residms;
    uint8_t  selrecordls;
    uint8_t  selrecordms;
    uint8_t  offsetls;
    uint8_t  offsetms;
    uint8_t  progress;
}  __attribute__ ((packed)) ;



#endif
