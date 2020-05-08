project('openpower-libhei', 'cpp',
        version: '0.1', meson_version: '>=0.50.0',
        default_options: [
            'warning_level=3',
            'werror=true',
            'cpp_std=c++14',
            'cpp_args=-Wno-unused-parameter'
       ])

incdir = include_directories('src')

libhei_src = ['src/isolator/hei_isolator.cpp',
              'src/isolator/hei_isolation_node.cpp',
              'src/register/hei_hardware_register.cpp',
              'src/util/hei_bit_string.cpp']

libhei_dep = declare_dependency(include_directories : incdir,
                                sources : libhei_src)

# build static library libhei.a (note that the libray name is hei, the
# resulting filename will be libhei.a)
libhei_static = static_library('hei',
                                dependencies: libhei_dep,
                                install: true)

install_headers('src/hei_chip.hpp',
                'src/hei_includes.hpp',
                'src/hei_isolation_data.hpp',
                'src/hei_macros.hpp',
                'src/hei_main.hpp',
                'src/hei_types.hpp',
                'src/hei_user_interface.hpp',
                subdir : 'libhei')

install_headers('src/isolator/hei_signature.hpp',
                'src/isolator/hei_isolator.hpp',
                'src/isolator/hei_isolation_node.hpp',
                subdir : 'libhei/isolator')

install_headers('src/register/hei_hardware_register.hpp',
                'src/register/hei_operator_register.hpp',
                'src/register/hei_scom_register.hpp',
                'src/register/hei_register.hpp',
                subdir : 'libhei/register')

install_headers('src/util/hei_bit_string.hpp',
                'src/util/hei_flyweight.hpp',
                subdir : 'libhei/util')

pkg_mod = import('pkgconfig')

pkg_mod.generate(libraries : libhei_static,
                 version : '0.1',
                 name : 'libhei',
                 subdirs: 'libhei',
                 filebase : 'hei',
                 description : 'Openpower Hardware Error Isolator')

build_tests = get_option('tests')

if not build_tests.disabled()
  subdir('test')
endif
