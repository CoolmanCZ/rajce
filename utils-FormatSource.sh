#!/bin/bash

path="app/Rajce"
files=`find ${path} -type f -name "*.cpp" -o -name "*.h"`

for i in ${files}; do
	if [[ "${i}" =~ "Common.h" ]]; then
		continue;
	fi
	#echo "${i}"
	clang-format -style="file" -i "${i}"
done

