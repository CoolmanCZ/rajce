#!/bin/bash

path="app/Rajce"
files=`ls ${path}/*.{h,cpp}`

for i in ${files}; do
	if [[ "${i}" =~ "Common.h" ]]; then
		continue;
	fi
	#echo "${i}"
	clang-format -style="{BasedOnStyle: LLVM, IndentWidth: 4, TabWidth: 4, UseTab: Always, SortIncludes: false, ColumnLimit: 100}" -i "${i}"
done

