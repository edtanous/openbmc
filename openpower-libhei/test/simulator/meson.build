# Simulator sources
sim_libhei = [
    'simulator.cpp',
    'hei_user_interface.cpp',
    '../hei_user_defines.cpp'
]

# Test cases
test_libhei = [
    'sample_test_case.cpp',
]

gtest = dependency('gtest', main : true, required : false, method : 'system')

# Build simulator linked locally
if gtest.found()
    test('simulator',
         executable('simulator', sim_libhei, test_libhei,
                    dependencies : gtest,
                    link_with : libhei_static,
                    include_directories: incdir))
endif
