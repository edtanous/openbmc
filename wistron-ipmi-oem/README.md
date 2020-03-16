# wistron-ipmi-oem

## Supported Commands
- Detect the presence of Riser-F

## Command Documentation

### Detect the presence of Riser-F
Netfun: 0x32
Command: 0x01

On the mihawk platform, different riser cards can be supported. In order to be
able to distinguish the currently used riser cards, we design an ipmi-oem
command to detect riser cards.

If the current riser-F is used, i2c command will get a data. If other riser
cards are used, the i2c command will not get the data and will get a negative
return value.

