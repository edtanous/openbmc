proto = custom_target(
  'proto',
  command: [
    find_program('protoc', native: true),
    '--proto_path=@CURRENT_SOURCE_DIR@',
    '--cpp_out=@OUTDIR@',
    '@INPUT@'
  ],
  output: [
    'binaryblob.pb.cc',
    'binaryblob.pb.h',
  ],
  input: 'binaryblob.proto')
