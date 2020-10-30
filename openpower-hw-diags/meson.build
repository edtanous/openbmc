# See README.md for details.
project('openpower-hw-diags', 'cpp',
        version: '0.1', meson_version: '>=0.49.0',
        default_options: [
          'warning_level=3',
          'werror=true',
          'cpp_std=c++17',
          'cpp_args=-Wno-unused-parameter'
        ])

# First, look if the libhei library has already been built and installed. If
# not, default to the subproject.
libhei_dep = dependency('hei', fallback : ['libhei', 'libhei_dep'])

sdbusplus_dep       = dependency('sdbusplus', version : '>=1.0')
dbus_interfaces_dep = dependency('phosphor-dbus-interfaces')

incdir = include_directories('.')

# See if phosphor-logging is available, if not use test case logging code. This
# allows for local builds outside of CI test sandbox.
h = 'phosphor-logging/log.hpp'
cmplr = meson.get_compiler('cpp')
if cmplr.compiles('#include <@0@>'.format(h), name : '#include <@0@>'.format(h))
    test_arg = []
    phosphor_logging = true
else
    test_arg = [
        '-DTEST_TRACE',
    ]
    phosphor_logging = false
endif

subdir('analyzer')
subdir('attn')

pthread = declare_dependency(link_args : '-pthread')
lrt = declare_dependency(link_args : '-lrt')

no_listener_mode = get_option('nlmode')

if not no_listener_mode.disabled()
  executable('openpower-hw-diags', 'main_nl.cpp', 'cli.cpp',
              link_with : [analyzer, attn],
              install : true)
else
  executable('openpower-hw-diags', 'main.cpp', 'cli.cpp', 'listener.cpp',
              dependencies : [lrt, pthread],
              link_with : [analyzer, attn],
              cpp_args : test_arg,
              install : true)
endif

build_tests = get_option('tests')

if not build_tests.disabled()
  subdir('test')
endif
