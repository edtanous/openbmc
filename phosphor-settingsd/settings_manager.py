#!/usr/bin/python -u

import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import os
import os.path as path
import Openbmc
import settings_file as s

DBUS_NAME = 'org.openbmc.settings.Host'
OBJ_NAME = '/org/openbmc/settings/host0'
CONTROL_INTF = 'org.openbmc.Settings'

class HostSettingsObject(Openbmc.DbusProperties):
    def __init__(self, bus, name, settings, path):
        Openbmc.DbusProperties.__init__(self)
        dbus.service.Object.__init__(self, bus, name)

        self.path = path
        if not os.path.exists(path):
            os.mkdir(path)

        # Listen to changes in the property values and sync them to the BMC
        bus.add_signal_receiver(self.settings_signal_handler,
            dbus_interface = "org.freedesktop.DBus.Properties",
            signal_name = "PropertiesChanged",
            path = "/org/openbmc/settings/host0")

        # Create the dbus properties
        for i in settings['host'].iterkeys():
            shk = settings['host'][i]
            self.set_settings_property(shk['name'],
                                       shk['type'],
                                       shk['default'])

    def get_bmc_value(self, name):
        try:
            with open(path.join(self.path, name), 'r') as f:
                return f.read()
        except (IOError):
            pass
        return None

    # Create dbus properties based on bmc value. This will be either a value
    # previously set, or the default file value if the BMC value does not exist.
    def set_settings_property(self, name, type, value):
        bmcv = self.get_bmc_value(name)
        if bmcv:
            value = bmcv
        if type=="i":
            self.Set(DBUS_NAME, name, value)
        elif type=="s":
            self.Set(DBUS_NAME, name, str(value))

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
    def settings_signal_handler(self, interface_name, changed_properties, invalidated_properties):
        for name, value in changed_properties.items():
            self.set_system_settings(name, value)

    # Placeholder signal. Needed to register the settings interface.
    @dbus.service.signal(DBUS_NAME, signature='s')
    def SettingsUpdated(self, sname):
        pass

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = HostSettingsObject(bus, OBJ_NAME, s.SETTINGS, "/var/lib/obmc/")
    mainloop = gobject.MainLoop()

    print "Running HostSettingsService"
    mainloop.run()

