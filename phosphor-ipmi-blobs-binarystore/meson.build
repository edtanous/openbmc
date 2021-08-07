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

binarystoreblob_pre = declare_dependency(
  include_directories: [
    include_directories('.'),
    include_directories('proto')],
  dependencies: [
    protobuf_dep,
    ipmi_blob_dep,
    phosphor_logging_dep,
  ])

binarystoreblob_lib = library(
  'binarystore',
  'binarystore.cpp',
  'sys.cpp',
  'sys_file_impl.cpp',
  'handler.cpp',
  proto,
  implicit_include_directories: false,
  dependencies: binarystoreblob_pre,
  install: true,
  install_dir: get_option('libdir') / 'blob-ipmid')

binarystoreblob_dep = declare_dependency(
  link_with: binarystoreblob_lib,
  dependencies: binarystoreblob_pre)

if not get_option('blobtool').disabled()
  executable(
    'blobtool',
    'blobtool.cpp',
    implicit_include_directories: false,
    dependencies: binarystoreblob_dep,
    install: true)
endif


if not get_option('tests').disabled()
  subdir('test')
endif
