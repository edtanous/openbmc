# Source files.
util_src = files(
    'data_file.cpp',
    'dbus.cpp',
    'ffdc_file.cpp',
    'pdbg.cpp',
    'temporary_file.cpp',
)

# Library dependencies.
util_deps = [
    libhei_dep,
    libpdbg_dep,
]

if get_option('phal').enabled()
    util_deps += dependency('libdt-api')
endif

# Create static library.
util_lib = static_library(
    'util_lib',
    util_src,
    include_directories : incdir,
    dependencies : util_deps,
    cpp_args : test_arg,
    install : false,
)

