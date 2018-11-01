This .so file is designed to support the OpenPOWER's BIOS OEM commands.
Documentation can be found by contacting the OpenPOWER mailing list @ https://github.com/open-power/op-build

## To Build
```
To build this package, do the following steps:

    1. ./bootstrap.sh
    2. ./configure ${CONFIGURE_FLAGS}
    3. make

To clean the repository run `./bootstrap.sh clean`.
```

## Supported Commands
- Partial Add
- Prepare for host update
- BMC Factory Reset

## Command Documentation

### BMC Factory Reset
Netfun: 0x3a
Command: 0x11

This command will call to reset the BMC to its factory default. See [here][0]
for the factory reset implementation details.

This includes:
1. Power the chassis off. The host needs to be powered off because the factory
reset deletes the inventory items, which are needed for the BMC during the power
on path, causing the power on to fail. The inventory items are repopulated
during a host power on.
An enhancement to OpenBMC would be to handle missing inventory items during
a BMC reboot when the host is on.
2. Set the BMC to perform factory reset on reboot.
3. Reboot the BMC for the factory reset to take effect.

Because the chassis is powered off, the host does not receive a return code
when successful.

This command is not allowed when the IPMI restriction mode is set to
'Whitelist'.  See [here][1] for more information.

[0]: https://github.com/openbmc/phosphor-dbus-interfaces/tree/master/xyz/openbmc_project/Common/FactoryReset#xyzopenbmc_projectsoftwarebmcupdater
[1]: https://github.com/openbmc/phosphor-dbus-interfaces/blob/master/xyz/openbmc_project/Control/Security/RestrictionMode.interface.yaml

