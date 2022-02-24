src_pb = custom_target(
  'proto',
  command: [
    find_program('protoc', native: true),
    '--proto_path=@CURRENT_SOURCE_DIR@',
    '--cpp_out=@OUTDIR@',
    '@INPUT@'
  ],
  output: [
    'binaryblob.pb.h',
    'binaryblob.pb.cc',
  ],
  input: 'binaryblob.proto')

binaryblobproto_pre = declare_dependency(
  include_directories: include_directories('.'),
  dependencies: dependency('protobuf'))

binaryblobproto_lib = static_library(
  'binaryblob_proto',
  src_pb[1],
  implicit_include_directories: false,
  dependencies: binaryblobproto_pre)

binaryblobproto_dep = declare_dependency(
  dependencies: binaryblobproto_pre,
  link_with: binaryblobproto_lib,
  sources: src_pb[0])
