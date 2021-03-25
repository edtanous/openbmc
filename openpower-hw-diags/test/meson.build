test_arg = [
    '-DTEST_TRACE',
]

# end2end code exerciser for experiment and testing
subdir('end2end')

tests = [
  'bin_stream_test',
  'ffdc_file_test',
  'resolution_test',
  'service_data_test',
]

gtest = dependency('gtest', main : true, required : false, method : 'system')

if gtest.found()
    foreach t : tests
        test(t, executable(t.underscorify(), t + '.cpp',
                           link_with : [ util_lib ],
                           dependencies : [ libhei_dep, gtest ],
                           cpp_args : test_arg,
                           include_directories : incdir))
    endforeach
endif
