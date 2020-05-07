# build simulator
subdir('simulator')

# supporting files that need compiled/linked
test_src = ['../src/util/hei_bit_string.cpp',
               'hei_user_defines.cpp']

# build g-test framework unit tests
gtests = [
    'bit_string_test',
    'flyweight_test'
]

gtest = dependency('gtest', main : true, required : false, method : 'system')

if gtest.found()
    foreach g : gtests
        test(g, executable(g.underscorify(), g + '.cpp', test_src, \
                            dependencies : gtest, \
                            include_directories : incdir))
  endforeach
endif

# build unit-test framework
tests = ['hei_operator_register_testcase']

foreach t : tests
        test(t, executable(t.underscorify(), t + '.cpp', test_src, \
                            include_directories : incdir))
endforeach
