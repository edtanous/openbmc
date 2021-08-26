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

### BIOS Related Commands (0x7X)

#### BIOS Boot Count 0x71

This is a 32 bits register that provides bios pushs the boot counter to BMC, or
get the boot counter from BMC.

Request
|Byte(s) |Value|Description
|--------|-----|----
|0x00|0x71|Subcommand
|0x01|Op value|Operation to be performed
|0x02-0x05|byte0 byte1 byte2 byte3|32-bit Count value for set operation

|Op value |Operation to be performed
|------|---------
|0x00|Read boot count value
|0x01|Increment boot count value
|0x02|Clear boot count value
|0x03|Set the boot count with given 4 byte value

Response
|Byte(s) |Value  |Description
|--------|-------|----
|0x00|CC|Completion code returned
|0x01|--|Byte 0 of boot count
|0x02|--|Byte 1 of boot count
|0x03|--|Byte 2 of boot count
|0x04|--|Byte 3 of boot count

Completion Codes (CC)
|CC   |Description
|-----|-----------|
|0xC1|Invalid Command
|0x00|Command Success
|0xC7|Data length Invalid

Example
|# |Command |Example
|--------|-------|----
|1|Read the boot count|ipmitool raw 0x34 0x71 0x00
|2|Increment the boot count by 1|ipmitool raw 0x34 0x71 0x01
|3|Clear the boot count to all 0's|ipmitool raw 0x34 0x71 0x02
|4|Set the boot count to given value|ipmitool raw 0x34 0x71 0x03 byte0 byte1 byte2 byte3
