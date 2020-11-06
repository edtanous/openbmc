# gather analyzer sources to be used here and elsewhere if needed
analyzer_src = files(
    'analyzer_main.cpp',
    'hei_user_interface.cpp',
    'initialize_isolator.cpp',
    '../util/ffdc_file.cpp',
    '../util/pdbg.cpp',
    '../util/temporary_file.cpp',
)

# Library dependencies.
analyzer_deps = [
    dbus_interfaces_dep,
    libhei_dep,
    sdbusplus_dep,
]

# Create hardware error analyzer library
analyzer = static_library('analyzer',
                          analyzer_src,
                          include_directories : incdir,
                          dependencies : analyzer_deps,
                          cpp_args : test_arg,
                          install : false)
