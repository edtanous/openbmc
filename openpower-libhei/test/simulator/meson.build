sim_sources = [
    'hei_sim_main.cpp',
    '../../src/isolator/hei_isolator.cpp',
    '../../src/util/hei_bit_string.cpp',
]

sim = executable('simulator', sim_sources, \
                    include_directories: incdir)

test('simulator', sim)

