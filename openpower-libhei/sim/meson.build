tests = ['openpower-libhei-sim']

sim_sources = ['hei_sim_main.cpp',
                '../src/hei_isolator.cpp']

foreach t : tests
  test(t, executable(t.underscorify(), sim_sources, include_directories : incdir))
endforeach
