project(
  'phosphor-ipmi-blobs-binarystore',
  'cpp',
  version: '0.1',
  meson_version: '>=0.57.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=3',
    'werror=true',
  ]
)

subdir('proto')
subdir('include')
subdir('src')

if not get_option('tests').disabled()
  subdir('test')
endif
