/********************************************************************************
*                       HON HAI Precision IND.Co., LTD.                         *
*            Personal Computer & Enterprise Product Business Group              *
*                      Enterprise Product Business Gro:qup                      *
*                                                                               *
*     Copyright (c) 2010 by FOXCONN/CESBG/CABG/SRD. All rights reserved.        *
*     All data and information contained in this document is confidential       *
*     and proprietary information of FOXCONN/CESBG/CABG/SRD and all rights      *
*     are reserved. By accepting this material the recipient agrees that        *
*     the information contained therein is held in confidence and in trust      *
*     and will not be used, copied, reproduced in whole or in part, nor its     *
*     contents revealed in any manner to others without the express written     *
*     permission of FOXCONN/CESBG/CABG/SRD.                                     *
*                                                                               *
********************************************************************************/

#include <ipmid/api.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/message/types.hpp>
#include <bioscommands.hpp>
#include <boost/endian/arithmetic.hpp>
#include <file_handling.hpp>

struct bios_boot_count
{
    uint32_t header;
    uint32_t count;
};

namespace ipmi
{
static void registerBIOSFunctions() __attribute__((constructor));
ipmi::RspType<uint32_t> FiiBIOSBootCount(boost::asio::yield_context yield,
        std::vector<uint8_t> reqParams)
{
    int boot_count_operation;
    bios_boot_count boot;
    if (reqParams.empty())
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
                " Fii bios cmd : command format error.");
        return ipmi::responseReqDataLenInvalid();
    }

    boot_count_operation = reqParams[0];

    if((boot_count_operation == BOOT_COUNT_SET &&
        reqParams.size() != SET_BYTE_LENGTH) ||
        (boot_count_operation != BOOT_COUNT_SET &&
         reqParams.size() != OPERATION_BYTE_LENGTH))
    {
         return ipmi::responseReqDataLenInvalid();
    }

    if(boot_count_operation > BOOT_COUNT_SET)
    {
         return ipmi::responseInvalidCommand();
    }

    int fd = sysopen(EEPROM_PATH);
    readBin(fd, EEPROM_OFFSET, &boot, sizeof(boot));

    if(boot.header != BOOT_COUNT_HEADER)
    {
        phosphor::logging::log<phosphor::logging::level::INFO>(
                  "Boot count header is corrupted or missing. Initializing.");
        boot.header = BOOT_COUNT_HEADER;
        boot.count = INITIAL_VALUE;
        writeBin(fd, EEPROM_OFFSET, &boot, sizeof(boot));
    }

    switch(boot_count_operation)
    {
        case BOOT_COUNT_READ:
            break;
        case BOOT_COUNT_INCREMENT:
            boot.count = boot.count + 1;
            break;
        case BOOT_COUNT_CLEAR:
            boot.count = INITIAL_VALUE;
            break;
        case BOOT_COUNT_SET:
            memcpy(&boot.count, &reqParams[1], sizeof(boot.count));
            break;
    }

    if( boot_count_operation != BOOT_COUNT_READ )
    {
        writeBin(fd, EEPROM_OFFSET + 4, &boot.count, sizeof(boot.count));
    } 
    sysclose(fd);
    return ipmi::responseSuccess(boot.count);
}

void registerBIOSFunctions()
{
    std::fprintf(stderr, "Registering OEM:[0x34], Cmd:[%#04X] for Fii BIOS OEM Commands\n",
            FII_CMD_BIOS_BOOT_COUNT);
    ipmi::registerHandler(ipmi::prioOemBase, ipmi::netFnOemThree, FII_CMD_BIOS_BOOT_COUNT,
            ipmi::Privilege::User, FiiBIOSBootCount);
}
}
