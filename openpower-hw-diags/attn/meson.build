# needed to find external libraries not registered with package manager
cmplr = meson.get_compiler('cpp')

# dependency to link dbus support
sdbusplus = dependency('sdbusplus', version : '>=1.0')

# dependency to link libpdbg support
libpdbg = cmplr.find_library('pdbg')

# libpdbg requires linking with "whole-archive" option
whole_archive = declare_dependency(link_args : '-Wl,--whole-archive')
no_whole_archive = declare_dependency(link_args : '-Wl,--no-whole-archive')

executable('attn_handler',
           'attn_main.cpp', 'attn_handler.cpp',
           dependencies : [whole_archive, libpdbg,
                           no_whole_archive, sdbusplus],
           install : true)
