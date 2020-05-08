# needed to find external libraries not registered with package manager
cmplr = meson.get_compiler('cpp')

# async gpio monitor needs boost library
boost_args = ['-DBOOST_ASIO_DISABLE_THREADS',
              '-DBOOST_ERROR_CODE_HEADER_ONLY',
              '-DBOOST_SYSTEM_NO_DEPRECATED']

# dependency to link sdbusplus support
sdbusplus = dependency('sdbusplus', version : '>=1.0')

# dependency to link gpiod support
libgpiod = dependency('libgpiod', version : '>=1.4.1')

# dependency to link libpdbg support
libpdbg = cmplr.find_library('pdbg')

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
h = 'phosphor-logging/log.hpp'
if cmplr.compiles('#include <@0@>'.format(h), name : '#include <@0@>'.format(h))
    logging_src = 'attn_logging.cpp'
else
    logging_src = '../test/end2end/logging.cpp'
endif

# gather attention sources to be used here and elsewhere if needed
attn_src = files('attn_main.cpp', 'attn_handler.cpp', 'attn_monitor.cpp',
                 'bp_handler.cpp', 'ti_handler.cpp', logging_src,
                 'attention.cpp', 'attn_config.cpp')

# Create attention handler library
attn = static_library('attn_handler',
                      attn_src,
                      include_directories : incdir,
                      dependencies : [libpdbg, sdbusplus, libgpiod],
                      cpp_args : boost_args,
                      install : true)
