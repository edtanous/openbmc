project(
    'phosphor-ecc',
    'cpp',
    version: '1.0',
    default_options: [
        'cpp_std=c++17',
    ],
)

executable(
    'ecc_main',
    [
        'ecc_main.cpp',
        'ecc_manager.cpp',
    ],
    dependencies: [
        dependency('sdbusplus'),
        dependency('sdeventplus'),
        dependency('phosphor-dbus-interfaces'),
        dependency('phosphor-logging'),
    ],
    install: true,
    install_dir: get_option('bindir')
)

install_data(sources : 'maxlog.conf', install_dir : '/etc/ecc')

conf_data = configuration_data()
conf_data.set('BUSNAME', '"xyz.openbmc_project.memory.ECC"')
conf_data.set('OBJROOT', '"/xyz/openbmc_project/metrics/memory/"')
conf_data.set('OBJPATH', '"/xyz/openbmc_project/metrics/memory/BmcECC"')
conf_data.set('sysfsRootPath', '"/sys/devices/system/edac/mc/mc0/"')
conf_data.set('sysfsEDACReportPath', '"/sys/module/edac_core/parameters/edac_report"')

configure_file(output : 'config.h',
               configuration : conf_data)

