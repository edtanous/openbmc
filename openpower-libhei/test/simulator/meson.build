# Simulator sources
sim_src = [
    'simulator.cpp',
    'sim_hardware_access.cpp',
    '../sim_tracing.cpp'
]

# Test cases
test_src = [
    'sample_test_case.cpp',
]

gtest = dependency('gtest', main : true, required : false, method : 'system')

# Build simulator linked locally
if gtest.found()
    test('simulator',
         executable('simulator', sim_src, test_src,
                    dependencies : gtest,
                    link_with : libhei_static,
                    include_directories: incdir))
endif
