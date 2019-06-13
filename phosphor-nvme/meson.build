project(
    'phosphor-nvme',
    'cpp',
    version: '1.0',
    default_options: [
        'cpp_std=c++17',
    ],
)

executable(
    'nvme_main',
    [
        'nvme_main.cpp',
        'nvme_manager.cpp',
    ],
    dependencies: [
        dependency('phosphor-logging'),
        dependency('sdbusplus'),
        dependency('phosphor-dbus-interfaces'),
        dependency('sdeventplus'),
    ],
    install: true,
    install_dir: get_option('bindir')
)

conf_data = configuration_data()
conf_data.set('NVME_REQUEST_NAME', '"xyz.openbmc_project.nvme.manager"')
conf_data.set('NVME_OBJ_PATH_ROOT', '"/xyz/openbmc_project/sensors/temperature"')
conf_data.set('NVME_OBJ_PATH', '"/xyz/openbmc_project/sensors/temperature/nvme"')
conf_data.set('DBUS_PROPERTY_IFACE', '"org.freedesktop.DBus.Properties"')

configure_file(output : 'config.h',
               configuration : conf_data)
