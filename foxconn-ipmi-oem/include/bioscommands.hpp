/********************************************************************************
*                       HON HAI Precision IND.Co., LTD.                         *
*            Personal Computer & Enterprise Product Business Group              *
*                      Enterprise Product Business Group                        *
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

#pragma once

#define BOOT_COUNT_FILE "/etc/conf/bios.cnt"

enum fii_bios_cmds
{
    FII_CMD_BIOS_BOOT_COUNT = 0x71,
};

#define FII_CMD_BIOS_BOOT_COUNT_LEN 5
