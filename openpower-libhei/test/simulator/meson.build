# Simulator sources
sim_src = [
    'simulator.cpp',
    'user_interface.cpp',
]

# Isolator sources
iso_src = [
    '../../src/isolator/hei_isolator.cpp',
    '../../src/isolator/hei_isolation_chip.cpp',
    '../../src/isolator/hei_isolation_node.cpp',
    '../../src/register/hei_hardware_register.cpp',
    '../../src/util/hei_bit_string.cpp',
]

# Test cases
test_src = [
    'sample_test_case.cpp',
]

gtest = dependency('gtest', main : true, required : false, method : 'system')

if gtest.found()
    test('simulator', \
         executable('simulator', sim_src, iso_src, test_src, \
                    dependencies : gtest, include_directories: incdir))
endif

