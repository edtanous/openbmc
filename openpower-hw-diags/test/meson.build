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

# Compile the test dts into a binary for pdbg.
pdbg_test_dtb = custom_target('build_pdbg_test_dtb',
    input   : files('pdbg_test.dts'),
    output  : 'pdbg_test.dtb',
    command : [ find_program('dtc'), '-I', 'dts', '-O', 'dtb',
                '-o', '@OUTPUT@', '@INPUT@' ])

pdbg_env = 'PDBG_DTB=' + pdbg_test_dtb.full_path()

# end2end code exerciser for experiment and testing
subdir('end2end')

tests = [
  'bin_stream_test',
  'ffdc_file_test',
  'resolution_test',
  'pdbg_dts_test',
  'test-lpc-timeout',
  'test-pll-unlock',
]

analyzer_src = files(
  '../analyzer/plugins/p10-plugins.cpp',
  '../analyzer/service_data.cpp',
  '../analyzer/resolution.cpp',
)

# We cannot link to `util_lib` because that is built without `-DTEST_TRACE` and
# any of the util functions that use `trace.hpp` will throw a linker error
# because we don't have access to phosphor-logging in test ... yet.
util_src = files(
    '../util/ffdc_file.cpp',
    '../util/pdbg.cpp',
    '../util/temporary_file.cpp',
)

sim_src = files(
  'pdbg-sim-only.cpp',
)

foreach t : tests
    test(t,
         executable(t.underscorify(),
                    [ files(t + '.cpp'), pdbg_test_dtb,
                      analyzer_src, util_src, sim_src ],
                    dependencies : [ libhei_dep, libpdbg_dep, gtest_dep ],
                    cpp_args : test_arg,
                    include_directories : incdir),
         env : pdbg_env)
endforeach

################################################################################

tc = 'test-tod-step-check-fault'

src = [
  files(
    tc + '.cpp',
    '../analyzer/plugins/p10-tod-plugins.cpp',
    '../analyzer/service_data.cpp',
    '../util/pdbg.cpp',
  ),
  pdbg_test_dtb,
]

dep = [ libhei_dep, libpdbg_dep, gtest_dep ]

var = [ pdbg_env ]

exe = executable(tc.underscorify(), src, dependencies: dep,
                 cpp_args: test_arg, include_directories: incdir)

test(tc, exe, env: var)

