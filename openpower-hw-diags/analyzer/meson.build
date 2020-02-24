# Get opennpower-libhei dependencies. If not available via package manager
# "fallback" to using the local libhei subproject.
libhei_dep = dependency('libhei', fallback : ['libhei', 'libhei_dep'])

# Create hardware error analyzer library
analyzer = static_library('analyzer',
                          'analyzer_main.cpp',
                          'user_interface.cpp',
                          dependencies : libhei_dep,
                          install : true)
