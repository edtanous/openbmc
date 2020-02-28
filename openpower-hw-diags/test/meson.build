# end2end code exerciser for experiment and testing
subdir('end2end')

tests = [
  'hello-world',
]

foreach t : tests
  test(t, executable(t.underscorify(), t + '.cpp'))
endforeach
