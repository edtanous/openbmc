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

cpp = meson.get_compiler('cpp')
cpp.has_header('boost/endian/arithmetic.hpp')
cpp.has_header('nlohmann/json.hpp')
protobuf_dep = dependency('protobuf')
ipmi_blob_dep = dependency('phosphor-ipmi-blobs')
phosphor_logging_dep = dependency('phosphor-logging')

subdir('proto')
subdir('include')
subdir('src')

if not get_option('tests').disabled()
  subdir('test')
endif
