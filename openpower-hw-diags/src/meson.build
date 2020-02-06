# If libhei is not provided by the system then build using the subproject
libhei_dep = dependency('libhei', fallback : ['libhei', 'libhei_dep'])

# create openpower-hw-diags application
executable('openpower-hw-diags',
           'main.cpp',
           'user_interface.cpp',
           dependencies : libhei_dep,
           install : true)
