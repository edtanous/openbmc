# async gpio monitor needs boost library
boost_args = ['-DBOOST_ASIO_DISABLE_THREADS',
              '-DBOOST_ERROR_CODE_HEADER_ONLY',
              '-DBOOST_SYSTEM_NO_DEPRECATED']

# dependency to link gpiod support
libgpiod = dependency('libgpiod', version : '>=1.4.1')

# install systemd unit file
configure_file(
    input: 'attn_handler.service',
    output: 'attn_handler.service',
    copy: true,
    install_dir:
        dependency('systemd').get_pkgconfig_variable(
            'systemdsystemunitdir')
)

# see if phosphor-logging is available, if not use test case logging code
if (true == phosphor_logging)
    logging_src = 'attn_logging.cpp'
else
    logging_src = '../test/end2end/logging.cpp'
endif

# Source files.
attn_src = files(
    'attention.cpp',
    'attn_common.cpp',
    'attn_config.cpp',
    'attn_dbus.cpp',
    'attn_handler.cpp',
    'attn_main.cpp',
    'attn_monitor.cpp',
    'bp_handler.cpp',
    logging_src,
    'ti_handler.cpp',
    'vital_handler.cpp',
)

# for custom/raw PEL creation
pel_src = files(
    'pel/extended_user_header.cpp',
    'pel/pel_minimal.cpp',
    'pel/private_header.cpp',
    'pel/primary_src.cpp',
    'pel/user_header.cpp',
)

# Library dependencies.
attn_deps = [
    libgpiod,
    libpdbg_dep,
    sdbusplus_dep,
]

# Create static library.
attn_lib = static_library(
    'attn_lib',
    attn_src,
    pel_src,
    include_directories : incdir,
    dependencies : attn_deps,
    cpp_args : [boost_args, test_arg],
    install : false,
)
