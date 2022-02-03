# See README.md for details.
project('openpower-hw-diags', 'cpp',
        version: '0.1', meson_version: '>=0.57.0',
        default_options: [
          'warning_level=3',
          'werror=true',
          'cpp_std=c++20',
        ])

# Package directory root, which will contain required data files.
package_dir = join_paths( get_option('prefix'),
                          get_option('datadir'),
                          meson.project_name() )

# Compiler option so that source knows the package directory.
package_args = [ '-DPACKAGE_DIR="' + package_dir + '/"' ]

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

if get_option('phal').enabled()
  libphal_dep = cmplr.find_library('phal')
endif

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

# JSON parser
if cmplr.has_header('nlohmann/json.hpp')
    nlohmann_json_dep = declare_dependency()
else
    subproject('nlohmann', required: false)
    nlohmann_json_dep = declare_dependency(
        include_directories: [
            'subprojects/nlohmann/single_include',
            'subprojects/nlohmann/single_include/nlohmann',
        ]
    )
    nlohmann_json_dep = nlohmann_json_dep.as_system('system')
endif

# JSON validator
if cmplr.has_header('valijson/validator.hpp')
    valijson_dep = declare_dependency()
else
    subproject('valijson', required: false)
    valijson_dep = declare_dependency(
        include_directories: 'subprojects/valijson/include'
    )
    valijson_dep = valijson_dep.as_system('system')
endif

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
              sources : [ 'main_nl.cpp', 'cli.cpp', buildinfo, plugins_src ],
              dependencies : [ libhei_dep ],
              link_with : hwdiags_libs,
              install : true)
else
  executable('openpower-hw-diags',
              sources : [ 'main.cpp', 'cli.cpp', 'listener.cpp', buildinfo,
                          plugins_src ],
              dependencies : [lrt, pthread, libhei_dep],
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
