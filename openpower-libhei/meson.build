project('openpower-libhei', 'cpp',
        version: '0.1', meson_version: '>=0.49.0',
        default_options: [
            'warning_level=3',
            'werror=true',
            'cpp_std=c++11'
        ])

incdir = [include_directories('src')]

build_tests = get_option('tests')

if not build_tests.disabled()
  incdir += include_directories('test')
  subdir('test')
endif
