test_arg = [
    '-DTEST_TRACE',
]

# end2end code exerciser for experiment and testing
subdir('end2end')

tests = [
  'hello-world',
  'bin_stream_test',
  'resolution_test',
  'service_data_test',
]

gtest = dependency('gtest', main : true, required : false, method : 'system')

if gtest.found()
    foreach t : tests
        test(t, executable(t.underscorify(), t + '.cpp',
                           dependencies : [ libhei_dep, gtest ],
                           cpp_args : test_arg,
                           include_directories : incdir))
    endforeach

    test('ffdc_file_test',
         executable('ffdc_file_test', 'ffdc_file_test.cpp',
                    files('../util/ffdc_file.cpp',
                          '../util/temporary_file.cpp'),
                    dependencies : gtest,
                    cpp_args : test_arg,
                    include_directories : incdir))
endif
