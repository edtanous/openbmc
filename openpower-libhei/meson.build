# OpenBMC openpower-libhei project, see README.md for details.
project('openpower-libhei', 'cpp',
        version: '0.1', meson_version: '>=0.51.0',
        default_options: [
            'warning_level=3',
            'werror=true',
            'cpp_std=c++14',
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
# libhei library
#-------------------------------------------------------------------------------

incdir = include_directories('src')

libhei_src = [
    'src/chip_data/hei_chip_data.cpp',
    'src/isolator/hei_isolator.cpp',
    'src/isolator/hei_isolation_chip.cpp',
    'src/isolator/hei_isolation_node.cpp',
    'src/register/hei_hardware_register.cpp',
    'src/util/hei_bit_string.cpp',
]

libhei_dep = declare_dependency(include_directories : incdir,
                                sources : [libhei_src, buildinfo])

# build static library libhei.a (note that the libray name is hei, the
# resulting filename will be libhei.a)
libhei_static = static_library('hei',
                                dependencies: libhei_dep,
                                install: true)

install_headers(
    'src/hei_chip.hpp',
    'src/hei_isolation_data.hpp',
    'src/hei_main.hpp',
    'src/hei_signature.hpp',
    'src/hei_types.hpp',
    'src/hei_user_interface.hpp',
    'src/hei_util.hpp',
    subdir : 'libhei'
)

install_headers(
    'src/util/hei_bit_string.hpp',
    'src/util/hei_flyweight.hpp',
    subdir : 'libhei/util'
)

pkg_mod = import('pkgconfig')

pkg_mod.generate(libraries : libhei_static,
                 version : '0.1',
                 name : 'libhei',
                 subdirs: 'libhei',
                 filebase : 'hei',
                 description : 'Openpower Hardware Error Isolator')

#-------------------------------------------------------------------------------
# Chip Data Files
#-------------------------------------------------------------------------------

subdir('xml')

#-------------------------------------------------------------------------------
# Test
#-------------------------------------------------------------------------------

build_tests = get_option('tests')

if not build_tests.disabled()
  subdir('test')
endif

