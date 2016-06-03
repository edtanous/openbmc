#include "oemhandler.h"
#include <host-ipmid/ipmid-api.h>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <systemd/sd-bus.h>
#include <endian.h>

void register_netfn_oem_partial_esel() __attribute__((constructor));

const char *g_esel_path = "/tmp/esel";
uint16_t g_record_id = 0x0001;

///////////////////////////////////////////////////////////////////////////////
// For the First partial add eSEL the SEL Record ID and offset
// value should be 0x0000. The extended data needs to be in
// the form of an IPMI SEL Event Record, with Event sensor type
// of 0xDF and Event Message format of 0x04. The returned
// Record ID should be used for all partial eSEL adds.
//
// This function creates a /tmp/esel file to store the
// incoming partial esel.  It is the role of some other
// function to commit the error log in to long term
// storage.  Likely via the ipmi add_sel command.
///////////////////////////////////////////////////////////////////////////////
ipmi_ret_t ipmi_ibm_oem_partial_esel(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                     ipmi_request_t request, ipmi_response_t response,
                                     ipmi_data_len_t data_len, ipmi_context_t context)
{
	uint8_t *reqptr = (uint8_t *) request;
	esel_request_t esel_req;
	FILE *fp;
	int r = 0;
	uint8_t rlen;
	ipmi_ret_t rc = IPMI_CC_OK;
	const char *pio;

	esel_req.resid     = le16toh((((uint16_t) reqptr[1]) << 8) + reqptr[0]);
	esel_req.selrecord = le16toh((((uint16_t) reqptr[3]) << 8) + reqptr[2]);
	esel_req.offset    = le16toh((((uint16_t) reqptr[5]) << 8) + reqptr[4]);
	esel_req.progress  = reqptr[6];

	uint16_t used_res_id = get_sel_reserve_id();

	// According to IPMI spec, Reservation ID must be checked.
	if ( used_res_id != esel_req.resid ) {
		// 0xc5 means Reservation Cancelled or Invalid Reservation ID.
		printf("Used Reservation ID = %d\n", used_res_id);
		rc = IPMI_CC_INVALID_RESERVATION_ID;

		// clean g_esel_path. 
		r = remove(g_esel_path);
		if(r < 0)
			fprintf(stderr, "Error deleting %s\n", g_esel_path);

		return rc;
	}

    // OpenPOWER Host Interface spec says if RecordID and Offset are
	// 0 then then this is a new request
	if (!esel_req.selrecord && !esel_req.offset)
		pio = "wb";
	else
		pio = "rb+";

	rlen = (*data_len) - (uint8_t) (sizeof(esel_request_t));

	printf("IPMI PARTIAL ESEL for %s  Offset = %d Length = %d\n",
		g_esel_path, esel_req.offset, rlen);

	if ((fp = fopen(g_esel_path, pio)) != NULL) {
		fseek(fp, esel_req.offset, SEEK_SET);
		fwrite(reqptr+(uint8_t) (sizeof(esel_request_t)), rlen, 1, fp);
		fclose(fp);

		*data_len = sizeof(g_record_id);
		memcpy(response, &g_record_id, *data_len);
	} else {
		fprintf(stderr, "Error trying to perform %s for esel%s\n",pio, g_esel_path);
		rc = IPMI_CC_INVALID;
		*data_len     = 0;
	}

	// The first bit prepresents that this is the last partial packet
	// coming down.  If that is the case advance the record id so we
	// don't overlap logs.  This allows anyone to establish a log
	// directory system.
	if (esel_req.progress & 1 ) {
		g_record_id++;
	}

	return rc;
}

// Prepare for FW Update.
// Execute needed commands to prepare the system for a fw update from the host.
ipmi_ret_t ipmi_ibm_oem_prep_fw_update(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                                     ipmi_request_t request, ipmi_response_t response,
                                     ipmi_data_len_t data_len, ipmi_context_t context)
{
    ipmi_ret_t ipmi_rc = IPMI_CC_OK;
    *data_len = 0;

    int rc = 0;
    std::ofstream rwfs_file;
    const char  *busname = "org.openbmc.control.Bmc";
    const char  *objname = "/org/openbmc/control/bmc0";
    const char  *iface = "org.openbmc.control.Bmc";
    sd_bus *bus = ipmid_get_sd_bus_connection();
    sd_bus_message *reply = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int r = 0;

    // Set one time flag
    rc = system("fw_setenv openbmconce copy-files-to-ram copy-base-filesystem-to-ram");
    rc = WEXITSTATUS(rc);
    if (rc != 0) {
        fprintf(stderr, "fw_setenv openbmconce failed with rc=%d\n", rc);
        return IPMI_CC_UNSPECIFIED_ERROR; 
    }

    // Touch the image-rwfs file to perform an empty update to force the save
    // in case we're already in ram and the flash is the same causing the ram files
    // to not be copied back to flash
    rwfs_file.open("/run/initramfs/image-rwfs", std::ofstream::out | std::ofstream::app);
    rwfs_file.close();

    // Reboot the BMC for settings to take effect
    r = sd_bus_call_method(bus, busname, objname, iface,
                           "warmReset", &error, &reply, NULL);
    if (r < 0) {
        fprintf(stderr, "Failed to reset BMC: %s\n", strerror(-r));
        return -1;
    }
    printf("Warning: BMC is going down for reboot!\n");
    sd_bus_error_free(&error);
    reply = sd_bus_message_unref(reply);

    return ipmi_rc;
}

void register_netfn_oem_partial_esel()
{
	printf("Registering NetFn:[0x%X], Cmd:[0x%X]\n",NETFUN_OEM, IPMI_CMD_PESEL);
	ipmi_register_callback(NETFUN_OEM, IPMI_CMD_PESEL, NULL, ipmi_ibm_oem_partial_esel);

    printf("Registering NetFn:[0x%X], Cmd:[0x%X]\n", NETFUN_OEM, IPMI_CMD_PREP_FW_UPDATE);
    ipmi_register_callback(NETFUN_OEM, IPMI_CMD_PREP_FW_UPDATE, NULL, ipmi_ibm_oem_prep_fw_update);

	return;
}
