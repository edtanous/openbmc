# Get the gtest/gmock dependencies.
gtest_dep = dependency('gtest', main: true, disabler: true, required: false)
gmock_dep = dependency('gmock', disabler: true, required: false)
if not gtest_dep.found() or not gmock_dep.found()
    cmake = import('cmake')
    gtest_proj = cmake.subproject('googletest',
                                  required: false)
    if gtest_proj.found()
        gtest_dep = declare_dependency(
            dependencies: [
                dependency('threads'),
                gtest_proj.dependency('gtest'),
                gtest_proj.dependency('gtest_main'),
            ]
        )
        gmock_dep = gtest_proj.dependency('gmock')
    else
        assert(not get_option('tests').enabled(),
               'Googletest is required if tests are enabled')
    endif
endif

# build simulator
subdir('simulator')

# supporting files that need compiled/linked
test_src = ['../src/util/hei_bit_string.cpp',
            'sim_tracing.cpp']

# build g-test framework unit tests
gtests = [
    'bit_string_test',
    'flyweight_test',
    'operator_register_test',
    'test-util-hash',
]

foreach g : gtests
  test(g, executable(g.underscorify(), g + '.cpp', test_src,
                     dependencies : gtest_dep,
                     include_directories : incdir))
endforeach

