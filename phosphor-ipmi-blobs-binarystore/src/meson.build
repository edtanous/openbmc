binarystoreblob_pre = declare_dependency(
  include_directories: [
    include_directories('.'),
    blobstore_includes,
  ],
  dependencies: [
    protobuf_dep,
    ipmi_blob_dep,
    phosphor_logging_dep,
    binaryblobproto_dep,
  ]
)

binarystoreblob_lib = library(
  'binarystoreblob',
  'binarystore.cpp',
  'sys.cpp',
  'sys_file_impl.cpp',
  'handler.cpp',
  src_pb,
  implicit_include_directories: false,
  dependencies: binarystoreblob_pre,
)

binarystoreblob_dep = declare_dependency(
  link_with: binarystoreblob_lib,
  dependencies: binarystoreblob_pre
)

library(
  'binarystore',
  'main.cpp',
  implicit_include_directories: false,
  dependencies: binarystoreblob_dep,
  install: true,
  install_dir: get_option('libdir') / 'blob-ipmid'
)

if not get_option('blobtool').disabled()
  executable(
    'blobtool',
    'blobtool.cpp',
    implicit_include_directories: false,
    dependencies: binarystoreblob_dep,
    install: true
  )
endif
