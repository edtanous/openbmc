project(
    'openpower-proc-control',
    'cpp',
    default_options: [
        'warning_level=3',
        'werror=true',
        'cpp_std=c++17',
        'buildtype=debugoptimized',
        'b_ndebug=if-release',
    ],
    license: 'Apache-2.0',
    version: '1.0',
)
add_project_arguments('-Wno-psabi', language: 'cpp')

if get_option('cpp_std') != 'c++17'
    error('This project requires c++17')
endif

if(get_option('buildtype') == 'minsize')
    add_project_arguments('-DNDEBUG', language : 'cpp')
endif

cxx = meson.get_compiler('cpp')

extra_sources = []
extra_dependencies = []
extra_unit_files = []

# Configuration header file(config.h) generation
conf_data = configuration_data()

conf_data.set_quoted('DEVTREE_EXPORT_FILTER_FILE', get_option('DEVTREE_EXPORT_FILTER_FILE'),
                      description : 'Path to the phal devtree export filter file'
                    )

conf_data.set_quoted('DEVTREE_EXP_FILE', get_option('DEVTREE_EXP_FILE'),
                      description : 'Path to the devtree export copy file'
                    )

configure_file(configuration : conf_data,
               output : 'config.h'
              )

unit_subs = configuration_data()
unit_subs.set('bindir', join_paths(get_option('prefix'), get_option('bindir')))
unit_subs.set('ENABLE_PHAL_TRUE', '#')

if get_option('phal').enabled() and get_option('p9').enabled()
    error('phal and p9 cannot be selected at the same time')
endif

build_p9 = not get_option('p9').disabled()
build_openfsi = not get_option('openfsi').disabled()
build_phal = get_option('phal').enabled()

if get_option('phal').enabled() and not get_option('p9').disabled()
    build_p9 = false
endif

summary('building p9', build_p9)
summary('building openfsi', build_openfsi)
summary('building phal', build_phal)

if build_p9
    extra_sources += [
        'procedures/p9/cleanup_pcie.cpp',
        'procedures/p9/set_sync_fsi_clock_mode.cpp',
        'procedures/p9/start_host.cpp',
        'procedures/p9/start_host_mpreboot.cpp',
    ]
endif
if build_openfsi
    extra_sources += [
        'procedures/openfsi/scan.cpp',
    ]
endif
if build_phal
    extra_sources += [
        'procedures/phal/start_host.cpp',
        'procedures/phal/set_SPI_mux.cpp',
        'procedures/phal/proc_pre_poweroff.cpp',
        'procedures/phal/check_host_running.cpp',
        'procedures/phal/import_devtree.cpp',
        'extensions/phal/common_utils.cpp',
        'extensions/phal/pdbg_utils.cpp',
        'extensions/phal/create_pel.cpp',
        'extensions/phal/phal_error.cpp',
    ]
    extra_dependencies += [
        dependency('libdt-api'),
        cxx.find_library('ekb'),
        cxx.find_library('ipl'),
    ]
    extra_unit_files = [
        'service_files/set-spi-mux.service',
        'service_files/phal-reinit-devtree.service',
        'service_files/proc-pre-poweroff@.service',
        'service_files/op-reset-host-check@.service',
        'service_files/op-reset-host-clear.service',
        'service_files/phal-import-devtree@.service',
    ]
    unit_subs.set('ENABLE_PHAL_TRUE', '')
endif

executable(
    'openpower-proc-control',
    [
        'cfam_access.cpp',
        'ext_interface.cpp',
        'filedescriptor.cpp',
        'proc_control.cpp',
        'targeting.cpp',
        'procedures/common/cfam_overrides.cpp',
        'procedures/common/cfam_reset.cpp',
        'procedures/common/enter_mpreboot.cpp',
        'procedures/common/collect_sbe_hb_data.cpp',
        'util.cpp',
    ] + extra_sources,
    dependencies: [
        dependency('libgpiodcxx'),
        cxx.find_library('pdbg'),
        dependency('phosphor-dbus-interfaces'),
        dependency('phosphor-logging'),
        dependency('sdbusplus'),
        dependency('threads'),
        dependency('fmt'),
    ] + extra_dependencies,
    install: true
)

executable(
    'openpower-proc-nmi',
    [
        'nmi_main.cpp',
        'nmi_interface.cpp',
    ],
    dependencies: [
        cxx.find_library('pdbg'),
        dependency('phosphor-dbus-interfaces'),
        dependency('phosphor-logging'),
        dependency('sdbusplus'),
   ],
   install: true
)

if build_phal
    executable(
        'phal-export-devtree',
        [
            'extensions/phal/devtree_export.cpp',
            'extensions/phal/fw_update_watch.cpp',
            'extensions/phal/pdbg_utils.cpp',
            'extensions/phal/create_pel.cpp',
            'util.cpp',
        ],
        dependencies: [
            dependency('phosphor-logging'),
            dependency('sdbusplus'),
            dependency('sdeventplus'),
            dependency('fmt'),
            dependency('phosphor-dbus-interfaces'),
            cxx.find_library('pdbg'),
       ],
       install: true
   )
endif

unit_files = [
    'service_files/pcie-poweroff@.service',
    'service_files/xyz.openbmc_project.Control.Host.NMI.service',
    'service_files/op-stop-instructions@.service',
    'service_files/op-cfam-reset.service',
    'service_files/op-continue-mpreboot@.service',
    'service_files/op-enter-mpreboot@.service',
] + extra_unit_files

systemd_system_unit_dir = dependency('systemd').get_pkgconfig_variable(
    'systemdsystemunitdir',
    define_variable: ['prefix', get_option('prefix')])
foreach u : unit_files
    configure_file(
        configuration: unit_subs,
        input: u + '.in',
        install: true,
        install_dir: systemd_system_unit_dir,
        output: '@BASENAME@'
    )
endforeach

if not get_option('tests').disabled()
    test(
        'utest',
        executable(
            'utest',
            'test/utest.cpp',
            'targeting.cpp',
            'filedescriptor.cpp',
            dependencies: [
                dependency('gtest', main: true),
                dependency('phosphor-logging'),
            ],
            implicit_include_directories: false,
            include_directories: '.',
        )
    )
endif
