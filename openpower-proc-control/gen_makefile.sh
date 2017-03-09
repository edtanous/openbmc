#!/bin/bash

files=()
echo "openpower_procedures_cpp_files = \\"
for type in "$@";
do
    type=${type// /} #remove spaces
    for file in $(ls procedures/$type/*.cpp);
    do
        files+=($file)
    done
done

for file in ${files[@]};
do
    echo "	$file \\"
done
echo

cat << MAKEFILE
openpower_procedures.cpp: \$(openpower_procedures_cpp_files)
	cat \$^ > \$@

MAKEFILE
