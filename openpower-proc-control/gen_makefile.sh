#!/bin/bash

files=()
dir=$1     #Base directory
# shellcheck disable=SC2206 # $2 is purposefully wordsplit.
types=($2) #List of types (convert to array)

echo "openpower_procedures_cpp_files = \\"
for ((i=0; i<${#types[@]}; ++i));
do
    type=${types[$i]}
    type=${type// /} #remove spaces
    for file in "$dir"/procedures/"$type"/*.cpp;
    do
        files+=("$file")
    done
done

for file in "${files[@]}";
do
    echo "	$file \\"
done
echo

cat << MAKEFILE
openpower_procedures.cpp: \$(openpower_procedures_cpp_files)
	cat \$^ > \$@

MAKEFILE
