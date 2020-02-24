project('openpower-hw-diags', 'cpp',
        version: '0.1', meson_version: '>=0.49.0',
        default_options: [
          'warning_level=3',
          'werror=true',
          'cpp_std=c++17',
          'cpp_args=-Wno-unused-parameter'
        ])

# libhei is available as a subproject
subproject('libhei')

incdir = include_directories('.')

subdir('analyzer')
subdir('attn')

executable('openpower-hw-diags', 'main.cpp',
            link_with : [analyzer, attn],
            install : true)

build_tests = get_option('tests')

if not build_tests.disabled()
  subdir('test')
endif
