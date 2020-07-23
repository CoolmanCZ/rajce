#!/bin/bash

path="app/Rajce"
files=`find ${path} -type f -name "*.cpp" -o -name "*.h"`

for i in ${files}; do
	if [[ "${i}" =~ "Common.h" ]]; then
		continue;
	fi
	#echo "${i}"
	clang-format -style="{BasedOnStyle: LLVM, IndentWidth: 4, TabWidth: 4, UseTab: Always, SortIncludes: false, ColumnLimit: 100}" -i "${i}"
done

