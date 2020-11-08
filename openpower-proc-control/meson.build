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
        'phalerror/create_pel.cpp',
        'phalerror/phal_error.cpp',
    ]
    extra_dependencies += [
        dependency('libdt-api'),
        dependency('fmt'),
        cxx.find_library('ekb'),
        cxx.find_library('ipl'),
    ]
    extra_unit_files = [
        'set-spi-mux.service',
        'phal-reinit-devtree.service',
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
        'registration.cpp',
        'targeting.cpp',
        'procedures/common/cfam_overrides.cpp',
        'procedures/common/cfam_reset.cpp',
        'procedures/common/enter_mpreboot.cpp',
        'procedures/common/collect_sbe_hb_data.cpp',
    ] + extra_sources,
    dependencies: [
        dependency('libgpiodcxx'),
        cxx.find_library('pdbg'),
        dependency('phosphor-dbus-interfaces'),
        dependency('phosphor-logging'),
        dependency('sdbusplus'),
        dependency('threads'),
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

unit_files = [
    'pcie-poweroff@.service',
    'xyz.openbmc_project.Control.Host.NMI.service',
    'op-stop-instructions@.service',
    'op-cfam-reset.service',
    'op-continue-mpreboot@.service',
    'op-enter-mpreboot@.service',
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
        output: u
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
