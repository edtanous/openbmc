#include "oemhandler.h"
#include <host-ipmid/ipmid-api.h>
#include <stdio.h>
#include <string.h>

void register_netfn_oem_partial_esel() __attribute__((constructor));

const char *g_esel_path = "/tmp/";
uint16_t g_record_id = 0x0001;


///////////////////////////////////////////////////////////////////////////////
// For the First partial add eSEL the SEL Record ID and offset
// value should be 0x0000. The extended data needs to be in
// the form of an IPMI SEL Event Record, with Event sensor type
// of 0xDF and Event Message format of 0x04. The returned
// Record ID should be used for all partial eSEL adds.
//
// This function creates a /tmp/esel# file to store the
// incoming partial esel.  It is the role of some other
// function to commit the error log in to long term
// storage.  Likely via the ipmi add_sel command.
///////////////////////////////////////////////////////////////////////////////
ipmi_ret_t ipmi_ibm_oem_partial_esel(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                     ipmi_request_t request, ipmi_response_t response,
                                     ipmi_data_len_t data_len, ipmi_context_t context)
{
    esel_request_t *reqptr = (esel_request_t*) request;
    FILE *fp;
    // TODO: Issue 5: This is not endian-safe.
    short *recid  =  (short*) &reqptr->selrecordls;
    short *offset =  (short*) &reqptr->offsetls;
    uint8_t rlen;
    ipmi_ret_t rc = IPMI_CC_OK;
    char string[64];
    const char *pio;


    if (!*recid && !*offset) {
        // OpenPOWER Host Interface spec says if RecordID and Offset are
        // 0 then then this is a new request
        pio = "wb";
        snprintf(string, sizeof(string), "%s%s%04x", g_esel_path, "esel", g_record_id);
    } else {
        pio = "rb+";
        snprintf(string, sizeof(string), "%s%s%02x%02x", g_esel_path, "esel", reqptr->selrecordms, reqptr->selrecordls);
    }

    rlen = (*data_len) - (uint8_t) (sizeof(esel_request_t));


    printf("IPMI PARTIAL ESEL for %s  Offset = %d Length = %d\n",
        string, *offset, rlen);


    if ((fp = fopen(string, pio)) != NULL) {
        fseek(fp, *offset, SEEK_SET);
        fwrite(reqptr+1,rlen,1,fp);
        fclose(fp);

        *data_len = sizeof(g_record_id);
        memcpy(response, &g_record_id, *data_len);
    } else {
        fprintf(stderr, "Error trying to perform %s for esel%s\n",pio, string);
        rc = IPMI_CC_INVALID;
        *data_len     = 0;
    }

    // The first bit prepresents that this is the last partial packet
    // coming down.  If that is the case advance the record id so we
    // don't overlap logs.  This allows anyone to establish a log
    // directory system.
    if (reqptr->progress & 1 ) {
        g_record_id++;
    }

    return rc;
}


void register_netfn_oem_partial_esel()
{
    printf("Registering NetFn:[0x%X], Cmd:[0x%X]\n",NETFUN_OEM, IPMI_CMD_PESEL);
    ipmi_register_callback(NETFUN_OEM, IPMI_CMD_PESEL, NULL, ipmi_ibm_oem_partial_esel);
    return;
}
