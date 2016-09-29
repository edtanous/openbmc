#!/usr/bin/python -u

import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import os
import os.path as path
import sys
from obmc.dbuslib.bindings import DbusProperties, get_dbus
from IPy import IP

settings_file_path = os.path.join(
    sys.prefix,
    'share',
    'phosphor-settings')
sys.path.insert(1, settings_file_path)
import settings_file as s
import re

DBUS_NAME = 'org.openbmc.settings.Host'
OBJ_NAME = '/org/openbmc/settings/host0'
CONTROL_INTF = 'org.openbmc.Settings'


class HostSettingsObject(DbusProperties):
    def __init__(self, bus, name, settings, path):
        super(HostSettingsObject, self).__init__(conn=bus, object_path=name,
                                                validator=self.input_validator)
        self.bus = bus
        self.path = path
        # Needed to ignore the validation on default networkconfig values as
        # opposed to user giving the same.
        self.adminmode = True

        if not os.path.exists(path):
            os.mkdir(path)

        # Listen to changes in the property values and sync them to the BMC
        bus.add_signal_receiver(
            self.settings_signal_handler,
            dbus_interface="org.freedesktop.DBus.Properties",
            signal_name="PropertiesChanged",
            path="/org/openbmc/settings/host0")

        # Create the dbus properties
        for i in settings[DBUS_NAME].iterkeys():
            shk = settings[DBUS_NAME][i]
            self.set_settings_property(shk['name'],
                                       shk['type'],
                                       shk['default'])
        # Done with consuming factory settings.
        self.adminmode = False

    def get_bmc_value(self, name):
        try:
            with open(path.join(self.path, name), 'r') as f:
                return f.read()
        except (IOError):
            pass
        return None

    # Create dbus properties based on bmc value.
    # This will be either a value previously set,
    # or the default file value if the BMC value
    # does not exist.
    def set_settings_property(self, name, type, value):
        bmcv = self.get_bmc_value(name)
        if bmcv:
            value = bmcv
        if type == "i":
            self.Set(DBUS_NAME, name, int(value))
        elif type == "s":
            self.Set(DBUS_NAME, name, str(value))
        elif type == "b":
            self.Set(DBUS_NAME, name, bool(value))

    # Save the settings to the BMC. This will write the settings value in
    # individual files named by the property name to the BMC.
    def set_system_settings(self, name, value):
        bmcv = self.get_bmc_value(name)
        if bmcv != value:
            filepath = path.join(self.path, name)
            with open(filepath, 'w') as f:
                f.write(str(value))

    # Signal handler for when one ore more settings properties were updated.
    # This will sync the changes to the BMC.
    def settings_signal_handler(
            self, interface_name, changed_properties, invalidated_properties):
        for name, value in changed_properties.items():
            self.set_system_settings(name, value)

    # Placeholder signal. Needed to register the settings interface.
    @dbus.service.signal(DBUS_NAME, signature='s')
    def SettingsUpdated(self, sname):
        pass

    def validate_regex(self, regex, value):
        if not re.compile(regex).search(value):
            raise ValueError, "Invalid input. Data does not satisfy regex"

    def validate_range(self, min, max, value):
        if value not in range(min, max):
            raise ValueError, "Invalid input. Data not in allowed range"

    def validate_list_ignore_case(self, lst, value):
        if value.lower() not in map(lambda val: val.lower(), lst):
            raise ValueError, "Invalid input. Data not in allowed values"

    # validate host network configuration
    # need  "ipaddress=,prefix=,gateway=,mac=,addr_type="
    # Must be able to handle any order
    def validate_net_config(self, value):
        if self.adminmode:
            return

        # Need all of these to be given by the user.
        user_config = []
        all_config = ['ipaddress', 'prefix', 'gateway', 'mac', 'addr_type']

        # This has a hard data format mentioned above so no blanks allowed.
        if value.count(" ") or value.count("=") != 5:
            raise ValueError, "Invalid Network Data. No white spaces allowed"

        config = value.split(',')
        for key_value in config:
            key , value = key_value.split('=')
            if not key or not value:
                raise ValueError, "Invalid key or Data"

            # Add the current key seen so we can compare at the end to see
            # if all values have been given
            user_config.append(key.lower())

            if key.lower() == 'ipaddress' or key.lower() == 'gateway':
	        IP(value)

            elif key.lower() == 'mac':
                regex = '([a-fA-F0-9]{2}[:|\-]?){6}'
                self.validate_regex(regex, value)

            elif key.lower() == 'prefix':
                self.validate_range(0, 33, int(value))

            elif key.lower() == 'addr_type':
                allowed = ["STATIC", "DYNAMIC"]
                self.validate_list_ignore_case(allowed, value)

        # Did user pass everything ??
        if set(all_config) - set(user_config):
            raise ValueError, "Invalid Network Data. All information is mandatory"

    # Validate to see if the changes are in order
    def input_validator(self, iface, proprty, value):
        settings = s.SETTINGS
        shk = {}
        for key in settings[iface].iterkeys():
            if proprty == settings[iface][key]['name']:
                shk = settings[iface][key]
                break

        # User entered key is not present
        if not shk: raise KeyError, "Invalid Property"

        if shk['validation'] == 'list':
            self.validate_list_ignore_case(shk['allowed'], value)

        elif shk['validation'] == 'range':
            self.validate_range(shk['min'], shk['max']+1, value)

        elif shk['validation'] == 'regex':
            self.validate_regex(shk['regex'], value)

        elif shk['validation'] == 'custom':
	    getattr(self, shk['method'])(value)

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = get_dbus()
    obj = HostSettingsObject(bus, OBJ_NAME, s.SETTINGS, "/var/lib/obmc/")
    mainloop = gobject.MainLoop()

    obj.unmask_signals()
    name = dbus.service.BusName(DBUS_NAME, bus)
    print "Running HostSettingsService"
    mainloop.run()
