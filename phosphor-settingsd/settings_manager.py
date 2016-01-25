#!/usr/bin/python -u

import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import Openbmc
import settings_file as s

DBUS_NAME = 'org.openbmc.settings.Host'
OBJ_NAME = '/org/openbmc/settings/host0'
CONTROL_INTF = 'org.openbmc.Settings'

# TODO Save settings in tmp until persistant storage is available
# Path where the settings are stored in the BMC
SETTINGS_PATH = '/tmp/'

class HostSettingsObject(Openbmc.DbusProperties):
    def __init__(self,bus,name):
        Openbmc.DbusProperties.__init__(self)
        dbus.service.Object.__init__(self,bus,name)

        # Listen to changes in the property values and sync them to the BMC
        bus.add_signal_receiver(self.settings_signal_handler,
            dbus_interface = "org.freedesktop.DBus.Properties",
            signal_name = "PropertiesChanged",
            path = "/org/openbmc/settings/host0")

        # Create the dbus properties
        for i in s.SETTINGS['host'].iterkeys():
            self.sname = s.SETTINGS['host'][i]['name']
            self.stype = s.SETTINGS['host'][i]['type']
            self.svalue = s.SETTINGS['host'][i]['default']
            self.bmcvalue = self.svalue # Default BMC value to file value
            self.set_settings_property()

    # Check if the requested value is the same as the current one in the BMC
    def check_settings_need_update(self):
        filepath = SETTINGS_PATH + self.sname
        update = True
        try:
            with open(filepath, 'r') as f:
                self.bmcvalue = f.read() # Upate BMC value with value on system
                if self.bmcvalue == self.svalue:
                    update = False
        except (IOError):
            pass
        return update

    # Create dbus properties based on bmc value. This will be either a value
    # previously set, or the default file value if the BMC value does not exist.
    def set_settings_property(self):
        update = self.check_settings_need_update()
        if update == True:
            self.svalue = self.bmcvalue # Update svalue with the value that will be used 
            if self.stype=="i":
                self.Set(DBUS_NAME,self.sname,self.svalue)
            elif self.stype=="s":
                self.Set(DBUS_NAME,self.sname,str(self.svalue))

    # Save the settings to the BMC. This will write the settings value in
    # individual files named by the property name to the BMC.
    def set_system_settings(self):
        update = self.check_settings_need_update()
        if update == True:
            filepath = SETTINGS_PATH + self.sname
            with open(filepath, 'w') as f:
                f.write(str(self.svalue))

    # Signal handler for when one ore more settings properties were updated.
    # This will sync the changes to the BMC.
    def settings_signal_handler(self, interface_name, changed_properties, invalidated_properties):
        data = changed_properties                                           
        for i in data:                                                      
            self.sname = i
            self.svalue = data[i]
            self.set_system_settings()

    # Placeholder signal. Needed to register the settings interface.
    @dbus.service.signal(DBUS_NAME,signature='s')
    def SettingsUpdated(self,sname):
        pass

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)

    bus = Openbmc.getDBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = HostSettingsObject(bus, OBJ_NAME)
    mainloop = gobject.MainLoop()

    print "Running HostSettingsService"
    mainloop.run()

