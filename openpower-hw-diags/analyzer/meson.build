# Get opennpower-libhei dependencies. If not available via package manager
# "fallback" to using the local libhei subproject.
libhei_dep = dependency('libhei', fallback : ['libhei', 'libhei_dep'])

# gather analyzer sources to be used here and elsewhere if needed
analyzer_src = files('analyzer_main.cpp', 'user_interface.cpp')

# Create hardware error analyzer library
analyzer = static_library('analyzer',
                          analyzer_src,
                          dependencies : libhei_dep,
                          install : true)
