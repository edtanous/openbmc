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
CONTROL_INTF = 'org.openbmc.Settings'

def walk_nest(d, keys =()):
    """Arrange dictionary keys and values.

    Walk the dictionary and establish every possible path
    returned to and processed by 'create_object' below
    """
    if isinstance(d, dict):
        for k, v in d.iteritems():
            for rv in walk_nest(v, keys + (k, )):
                yield rv
    else:
        yield keys, d

def create_object(settings):
    """Create and format objects.

    Parse dictionary file and return all objects and main properties
    (name, type, default) for each attribute in the following format:
    [obj_name, attr_ name, attr_ type, default]
    """
    allobjects = {}
    for compound_key, val in walk_nest(settings):
        obj_name = compound_key[0].lower()
        obj_name = obj_name.replace(".","/")
        obj_name = "/" + obj_name + "0"

        for i in compound_key[1:len(compound_key)-2]:
            obj_name = obj_name + "/" + i

        setting = compound_key[len(compound_key) - 2]
        attribute = compound_key[len(compound_key) - 1]

        o = allobjects.setdefault(obj_name, {})
        s = o.setdefault(setting, {'name': None, 'type': None, 'default': None})
        s[attribute] = val

    return allobjects

class HostSettingsObject(DbusProperties):
    def __init__(self, bus, name, settings, path):
        super(HostSettingsObject, self).__init__(
            conn=bus,
            object_path=name,
            validator=self.input_validator)
        self.bus = bus
        self.path = path
        self.name = name
        self.settings = settings

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
            path=name)

        # Create the dbus properties
        for setting in settings.itervalues():
            self.set_settings_property(setting['name'],
                    setting['type'],
                    setting['default'])
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
    def set_settings_property(self, attr_name, attr_type, value):
        bmcv = self.get_bmc_value(attr_name)
        if bmcv:
            value = bmcv
        if attr_type == "i":
            self.Set(DBUS_NAME, attr_name, int(value))
        elif attr_type == "s":
            self.Set(DBUS_NAME, attr_name, str(value))
        elif attr_type == "b":
            self.Set(DBUS_NAME, attr_name, bool(value))

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
            raise ValueError("Invalid input. Data does not satisfy regex")

    def validate_range(self, min, max, value):
        if value not in range(min, max):
            raise ValueError("Invalid input. Data not in allowed range")

    def validate_list_ignore_case(self, lst, value):
        if value.lower() not in map(lambda val: val.lower(), lst):
            raise ValueError("Invalid input. Data not in allowed values")

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
            raise ValueError("Invalid Network Data. No white spaces allowed")

        config = value.split(',')
        for key_value in config:
            key, value = key_value.split('=')
            if not key or not value:
                raise ValueError("Invalid key or Data")

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
            raise ValueError(
                "Invalid Network Data. All information is mandatory")

    # Validate to see if the changes are in order
    def input_validator(self, iface, proprty, value):
        # User entered key is not present
        shk = None
        for attr in self.settings.itervalues():
            if attr['name'] == proprty:
                shk = attr

        if shk is None:
            raise KeyError("Invalid Property")

        validation = shk.get('validation', None)

        if validation == 'list':
            self.validate_list_ignore_case(shk['allowed'], value)

        elif validation == 'range':
            self.validate_range(shk['min'], shk['max']+1, value)

        elif validation == 'regex':
            self.validate_regex(shk['regex'], value)

        elif validation == 'custom':
            getattr(self, shk['method'])(value)

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = get_dbus()
    allobjects = create_object(s.SETTINGS)
    lastobject = None
    objs = []
    for o, settings in allobjects.iteritems():
        objs.append(HostSettingsObject(bus, o, settings, "/var/lib/obmc/"))
        objs[-1].unmask_signals()

    mainloop = gobject.MainLoop()
    name = dbus.service.BusName(DBUS_NAME, bus)
    print "Running HostSettingsService"
    mainloop.run()

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
