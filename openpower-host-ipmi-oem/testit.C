#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <host-ipmid/ipmid-api.h>
#include "oemhandler.h"

using namespace std;

const char* g_filepath = "/tmp/";

const char * getFilePath(void) {    return g_filepath; }

// Number of bytes without the IPMI completion code
#define MAXRESPONSE 2



// Returns the length of the file
std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}


// Compares a string to the data in a file.
// Returns 0 if complete match
int compareData(const char *filename, const char *string, size_t len)
{

    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    std::streamsize size =  in.tellg();

    std::vector<char>      buffer(size);

    in.read(buffer.data(), size);

    if (!std::memcmp(string, buffer.data(), len))
        return -1;


    return 0;
}


void test_multiwrite(unsigned int segment, const char *pString) {

    uint8_t request[1024];
    uint8_t response[MAXRESPONSE];
    size_t len, totalString;
    ipmi_ret_t rc;
    uint16_t i=0, j;

    esel_request_t  requestheader[]  = {0,0,0,0,0,0};
    esel_request_t  *pReqHdr = requestheader;
    uint8_t firstime[] = { 0x00, 0x00, 0xdf, 0x00, 0x00, 0x00, 0x00, 0x20,
                           0x00, 0x04, 0x0c, 0x1e, 0x07, 0xaa, 0x00, 0x00};

    ipmi_request_t  pRequest  = request;
    ipmi_response_t pResponse = response;
    ipmi_data_len_t pLen      = &len;

    totalString = strlen(pString);

    std::memcpy(pRequest, requestheader, sizeof(requestheader));
    std::memcpy(&request[sizeof(requestheader)], firstime, sizeof(firstime));

    *pLen = sizeof(requestheader) + sizeof(firstime);

    rc = ipmi_ibm_oem_partial_esel(0x3E, 0xF0, pRequest, pResponse, pLen, NULL);
    if (rc  != IPMI_CC_OK) { printf("Error completion code returned %d\n", rc);}
    if (len != 2)          { printf("Error data buffer length failed len\n");}

    pReqHdr->selrecordls = response[0];
    pReqHdr->selrecordms = response[1];


    for (i=0; i<totalString; i+=segment) {

        pReqHdr->offsetls = (i&0x00FF);
        pReqHdr->offsetms = ((i&0xFF00) >> 8);

        // printf("Record id msls 0x%02x%02x\n", pReqHdr->selrecordms, pReqHdr->selrecordls);
        // printf("Offset 0x%04x , msls = 0x%02x%02x\n", i, pReqHdr->offsetms , pReqHdr->offsetls);

        if (i+segment > totalString) {
            j = totalString-i;
        } else {
            j = segment;
        }

        std::memcpy(pRequest, requestheader, sizeof(requestheader));
        std::memcpy(&request[sizeof(requestheader)], pString+i, j);
        len = sizeof(*requestheader) + j;

        rc = ipmi_ibm_oem_partial_esel(0x3E, 0xF0, pRequest, pResponse, pLen, NULL);

        if (rc  != IPMI_CC_OK) { printf("Error completion code returned %d\n", rc);}
        if (len != 2)          { printf("Error data buffer length failed\n");}

        pReqHdr->selrecordls = response[0];
        pReqHdr->selrecordms = response[1];


    }


    if (filesize("/tmp/esel0100") !=  (unsigned int) strlen(pString)) { printf("Error fileszie mismatch\n");}

    // /tmp/esel000 should be identical to the incoming string
    rc = compareData("/tmp/esel0100",pString,strlen(pString));
    if (rc != 0) {printf("Data miscompare %d\n",rc);}

    return;
}



// Method that gets called by shared libraries to get their command handlers registered
void ipmi_register_callback(ipmi_netfn_t netfn, ipmi_cmd_t cmd,
                       ipmi_context_t context, ipmid_callback_t handler)
{
}


int main()
{

    const char* shortstring = "C";
    const char* longstring  = "The President is very much a figurehead - he wields no real power whatsoever. He is apparently chosen by the government, but the qualities he is required to display are not those of leadership but those of finely judged outrage. For this reason the President is always a controversial choice, always an infuriating but fascinating character. His job is not to wield power but to draw attention away from it.";

    test_multiwrite(1,  shortstring);

    test_multiwrite(10, longstring);
    test_multiwrite(1, longstring);
    test_multiwrite(100, longstring);

    return 0;
}
