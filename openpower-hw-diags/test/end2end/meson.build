# create openpower-hw-diags executable for local testing
end2end = executable('openpower-hw-diags-test',
            'main.cpp', 'logging.cpp', 'bp_handler.cpp', 'ti_handler.cpp',
            '../../analyzer/analyzer_main.cpp', '../../cli.cpp',
            link_with : [analyzer, attn],
            include_directories : incdir,
            dependencies : libhei_dep,
            install : false)

#test('openpower-hw-diags-test', end2end)
