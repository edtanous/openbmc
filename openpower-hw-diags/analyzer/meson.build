# Source files.
analyzer_src = files(
    'analyzer_main.cpp',
    'create_pel.cpp',
    'hei_user_interface.cpp',
    'initialize_isolator.cpp',
    'resolution.cpp',
)

# Library dependencies.
analyzer_deps = [
    dbus_interfaces_dep,
    libhei_dep,
    sdbusplus_dep,
]

# Create static library.
analyzer_lib = static_library(
    'analyzer_lib',
    analyzer_src,
    include_directories : incdir,
    dependencies : analyzer_deps,
    cpp_args : test_arg,
    install : false,
)

# Install the RAS data files
subdir('ras-data')

