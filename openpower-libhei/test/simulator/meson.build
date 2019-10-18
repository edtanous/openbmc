sim_sources = [
    'hei_sim_main.cpp',
    'hei_sim_user_interface.cpp',
    '../../src/isolator/hei_isolator.cpp',
    '../../src/isolator/hei_isolation_node.cpp',
    '../../src/register/hei_hardware_register.cpp',
    '../../src/util/hei_bit_string.cpp',
]

sim = executable('simulator', sim_sources, \
                    include_directories: incdir)

test('simulator', sim)

