# Factory Reset

## Overview

The OpenBMC API defines a factory reset interface, which is intended to be used
to restore the BMC to its original manufacturer settings. This interface is
defined generically; it is specifically and variously implemented throughout
OpenBMC services, which allows these services to be individually restored to
factory defaults as needed.

More reading for OpenBMC services [here](https://github.com/openbmc/phosphor-dbus-interfaces/blob/master/xyz/openbmc_project/Common/FactoryReset/README.md).

## Known Implementations (listed by D-Bus service)

### org.open_power.Software.Host.Updater
Path: `/xyz/openbmc_project/software`
The host software updater factory reset immediately clears persistence files
and any data stored in the read-write and preserved volumes created by the host
BIOS.
