project('openpower-libhei', 'cpp',
        version: '0.1', meson_version: '>=0.50.0',
        default_options: [
            'warning_level=3',
            'werror=true',
            'cpp_std=c++11',
            'cpp_args=-Wno-unused-parameter'
       ])

incdir = [include_directories('src')]

# libhei_dep provides dependencies needed to build with libhei as a subproject
# rather than a base project.
libhei_src = ['src/isolator/hei_isolator.cpp',
              'src/isolator/hei_isolation_node.cpp',
              'src/register/hei_hardware_register.cpp',
              'src/util/hei_bit_string.cpp']

libhei_dep = declare_dependency(include_directories : incdir,
                                sources : libhei_src)

build_tests = get_option('tests')

if not build_tests.disabled()
  incdir += include_directories('test')
  subdir('test')
endif
