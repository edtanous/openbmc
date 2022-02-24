binarystoreblob_pre = declare_dependency(
  include_directories: blobstore_includes,
  dependencies: [
    dependency('phosphor-ipmi-blobs'),
    dependency('phosphor-logging'),
    binaryblobproto_dep,
  ])

binarystoreblob_lib = library(
  'binarystoreblob',
  'binarystore.cpp',
  'sys.cpp',
  'sys_file_impl.cpp',
  'handler.cpp',
  implicit_include_directories: false,
  dependencies: binarystoreblob_pre,
  install: true)

binarystoreblob_dep = declare_dependency(
  link_with: binarystoreblob_lib,
  dependencies: binarystoreblob_pre)

shared_module(
  'binarystore',
  'main.cpp',
  implicit_include_directories: false,
  dependencies: binarystoreblob_dep,
  install: true,
  install_dir: get_option('libdir') / 'blob-ipmid')

if not get_option('blobtool').disabled()
  executable(
    'blobtool',
    'blobtool.cpp',
    implicit_include_directories: false,
    dependencies: binarystoreblob_dep,
    install: true)
endif
