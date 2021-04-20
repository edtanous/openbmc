### Fii IPMI OEM Commands (0x34)

There are and will be a variety of sys specific commands.

### System Commands (0x h)

#### PCIe Bifurcation Command 0x03

The PCIe bifurcation command checks the type of pcei adapter instailled in
each slot.

Request

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x03|Subcommand

Response

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x03|Subcommand
|0x01|0x--|Bifurcation each bit identified the slot support x16(1b) or x8(0b)
|0x02|0x--|Present bit each bit identified PCIe adapter installed(0b) or not(1b)

Example
|# |Command |Example
|--------|-------|----
|1|Read the information|ipmitool 0x34 0x03

### BIOS Related Commands (7x h)

#### BIOS Boot Count 0x71

This is a 32 bits register that provides bios pushs the boot counter to BMC, or 
get the boot counter from BMC.

Request

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x03|Subcommand
|0x01|0x00/0x01| read/write the boot counter register
|0x02|0x--|Byte 0
|0x03|0x--|Byte 1
|0x04|0x--|Byte 2
|0x05|0x--|Byte 3

Response

|Byte(s) |Value  |Data
|--------|-------|----
|0x00|0x71|Subcommand
|0x00|0x--|Byte 0
|0x01|0x--|Byte 1
|0x02|0x--|Byte 2
|0x03|0x--|Byte 3

Example
|# |Command |Example
|--------|-------|----
|1|Read the boot count|ipmitool raw 0x34 0x71 0x00
|2|Add the boot count by 1|ipmitool raw 0x34 0x71 0x01
|3|Add the boot count by index|ipmitool raw 0x34 0x71 0x01 byte0 byte1 byte2 byte3
