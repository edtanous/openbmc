# See README.md for details.
project('openpower-hw-diags', 'cpp',
        version: '0.1', meson_version: '>=0.49.0',
        default_options: [
          'warning_level=3',
          'werror=true',
          'cpp_std=c++17',
        ])

#-------------------------------------------------------------------------------
# Versioning
#-------------------------------------------------------------------------------
buildinfo = vcs_tag(command: ['git', 'describe', '--always', '--long'],
                  input: 'buildinfo.hpp.in',
                  output: 'buildinfo.hpp',
                  replace_string:'@BUILDINFO@',
                  fallback: '0')

#-------------------------------------------------------------------------------
# Compiler
#-------------------------------------------------------------------------------

cmplr = meson.get_compiler('cpp')

#-------------------------------------------------------------------------------
# Config file
#-------------------------------------------------------------------------------

conf = configuration_data()

conf.set('CONFIG_PHAL_API', get_option('phal').enabled())

configure_file(input: 'config.h.in', output: 'config.h', configuration: conf)

#-------------------------------------------------------------------------------
# Include directories
#-------------------------------------------------------------------------------

# Only using the base directory. All header includes should provide the full
# path from the base directory.
incdir = include_directories('.')

#-------------------------------------------------------------------------------
# External library dependencies
#-------------------------------------------------------------------------------

# Look if the libhei library has already been built and installed. If not,
# default to the subproject.
libhei_dep = dependency('hei', fallback : ['libhei', 'libhei_dep'])

sdbusplus_dep       = dependency('sdbusplus', version : '>=1.0')
dbus_interfaces_dep = dependency('phosphor-dbus-interfaces')

libpdbg_dep = cmplr.find_library('pdbg')

# See if phosphor-logging is available, if not use test case logging code. This
# allows for local builds outside of CI test sandbox.
h = 'phosphor-logging/log.hpp'
if cmplr.compiles('#include <@0@>'.format(h), name : '#include <@0@>'.format(h))
    test_arg = []
    phosphor_logging = true
else
    test_arg = [
        '-DTEST_TRACE',
    ]
    phosphor_logging = false
endif

pthread = declare_dependency(link_args : '-pthread')
lrt = declare_dependency(link_args : '-lrt')

#-------------------------------------------------------------------------------
# Build the local static libraries
#-------------------------------------------------------------------------------

subdir('analyzer')
subdir('attn')
subdir('util')

hwdiags_libs = [
    analyzer_lib,
    attn_lib,
    util_lib,
]

#-------------------------------------------------------------------------------
# Build the executable
#-------------------------------------------------------------------------------

no_listener_mode = get_option('nlmode')

if not no_listener_mode.disabled()
  executable('openpower-hw-diags',
              sources : [ 'main_nl.cpp', 'cli.cpp', buildinfo ],
              link_with : hwdiags_libs,
              install : true)
else
  executable('openpower-hw-diags',
              sources : [ 'main.cpp', 'cli.cpp', 'listener.cpp', buildinfo ],
              dependencies : [lrt, pthread],
              link_with : hwdiags_libs,
              cpp_args : test_arg,
              install : true)
endif

#-------------------------------------------------------------------------------
# Test, if configured
#-------------------------------------------------------------------------------

build_tests = get_option('tests')

if not build_tests.disabled()
  subdir('test')
endif
