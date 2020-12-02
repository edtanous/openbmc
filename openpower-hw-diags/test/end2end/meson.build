# Source files specific to end2end test.
end2end_src = [
    'bp_handler.cpp',
    'logging.cpp',
    'main.cpp',
    'ti_handler.cpp',
]

# Additional source files needed for test.
additional_src = [
    '../../cli.cpp',
]

# create openpower-hw-diags executable for local testing
end2end = executable('openpower-hw-diags-test',
            end2end_src, additional_src,
            link_with : hwdiags_libs,
            include_directories : incdir,
            cpp_args : test_arg,
            install : false)

test('openpower-hw-diags-test', end2end)
