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

test_arg = [
    '-DTEST_TRACE',
]

# end2end code exerciser for experiment and testing
subdir('end2end')

tests = [
  'bin_stream_test',
  'ffdc_file_test',
  'resolution_test',
]

analyzer_src = files(
  '../analyzer/service_data.cpp',
)

foreach t : tests
    test(t, executable(t.underscorify(), [ t + '.cpp', analyzer_src ],
                       link_with : [ util_lib ],
                       dependencies : [ libhei_dep, gtest_dep ],
                       cpp_args : test_arg,
                       include_directories : incdir))
endforeach
