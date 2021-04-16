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

#include <common.hpp>
#include <bioscommands.hpp>

namespace ipmi
{
    static void registerBIOSFunctions() __attribute__((constructor));

    ipmi::RspType<std::vector<uint8_t>> FiiBIOSBootCount(boost::asio::yield_context yield, std::vector<uint8_t> reqParams)
    {
        bool op;
        std::vector<uint8_t> boot_count;
        uint32_t counter = 0, ret;

        if (reqParams.empty())
        {
            phosphor::logging::log<phosphor::logging::level::ERR>(" Fii bios cmd : command format error.");
            return ipmi::responseReqDataLenInvalid();
        }

        op = reqParams[0] & 0b11;
        // check the boot count file exist or not
        std::fstream fptr(BOOT_COUNT_FILE);

        if (!fptr.is_open())
        {
            std::cerr << " Fii bios cmd : file didn't exist and try to create one\n";
            ret = system("mkdir -p /etc/conf");
            std::ofstream outfile (BOOT_COUNT_FILE);
            outfile << "0" << std::endl;
            outfile.close();
            boot_count.push_back(static_cast<uint8_t>(counter));
            boot_count.push_back(static_cast<uint8_t>(counter >> 8));
            boot_count.push_back(static_cast<uint8_t>(counter >> 16));
            boot_count.push_back(static_cast<uint8_t>(counter >> 24));
            return ipmi::responseSuccess(boot_count);
        }
        else
        {
            std::string str;
            while (std::getline(fptr, str))
            {
                //boot_count.push_back(static_cast<uint8_t>(std::stoul(str)));
                counter = (std::stoul(str));
                //std::cerr << " Fii bios cmd : " << counter << std::endl;
            }
            boot_count.push_back(static_cast<uint8_t>(counter));
            boot_count.push_back(static_cast<uint8_t>(counter >> 8));
            boot_count.push_back(static_cast<uint8_t>(counter >> 16));
            boot_count.push_back(static_cast<uint8_t>(counter >> 24));
            fptr.close();
        }
        if (op == OP_CODE_READ)
        {
            return ipmi::responseSuccess(boot_count);
        }
        else if (op == OP_CODE_WRITE)
        {
            uint32_t value = 0;
            if (reqParams.size() == 1)
            {
                boot_count[0] += 1;
                value = boot_count[0] + (boot_count[1] << 8) + (boot_count[2] << 16) + (boot_count[3] << 24);
            }
            else if (reqParams.size() == FII_CMD_BIOS_BOOT_COUNT_LEN)
            {
                value = reqParams[1] + + (reqParams[2] << 8) + (reqParams[3] << 16) + (reqParams[4] << 24);
                boot_count.clear();
                boot_count.insert(boot_count.begin(), reqParams.begin()+1, reqParams.end());
            }
            std::ofstream fptr_w(BOOT_COUNT_FILE, std::ios::out | std::ios::trunc);
            fptr_w << value << std::endl;
            fptr_w.close();
        }
        else
        {
            return ipmi::responseInvalidCommand();
        }
        return ipmi::responseSuccess(boot_count);   
    }

    void registerBIOSFunctions()
    {
        std::fprintf(stderr, "Registering OEM:[0x34], Cmd:[%#04X] for Fii BIOS OEM Commands\n", FII_CMD_BIOS_BOOT_COUNT);
        ipmi::registerHandler(ipmi::prioOemBase, ipmi::netFnOemThree, FII_CMD_BIOS_BOOT_COUNT, ipmi::Privilege::User,
            FiiBIOSBootCount);

        return;
    }
}
