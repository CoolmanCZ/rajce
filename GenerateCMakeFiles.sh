#!/bin/bash
#
# Copyright (C) 2016 Radek Malcic
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

source upp_cmake/GenerateCMakeFiles-lib.sh

UPP_SRC_BASE="ultimatepp"
UPP_SRC_DIR="${UPP_SRC_BASE}/uppsrc"

PROJECT_NAME="Rajce/Rajce.upp"
PROJECT_FLAGS="-DflagGUI -DflagMT -DflagGCC -DflagLINUX -DflagPOSIX"

GENERATE_VERBOSE="1"
GENERATE_DEBUG="1"
GENERATE_PACKAGE="1"

generate_main_cmake_file "${PROJECT_NAME}" "${PROJECT_FLAGS}"

if [ "${GENERATE_DEBUG}" == "1" ]; then
    declare -A sorted_UPP_ALL_USES=$(printf "%s\n" "${UPP_ALL_USES[@]}" | sort -u);
    declare -A sorted_UPP_ALL_USES_DONE=$(printf "%s\n" "${UPP_ALL_USES_DONE[@]}" | sort -u);

    echo "Plugins used   : " ${sorted_UPP_ALL_USES[@]}
    echo "CMake generated: " ${sorted_UPP_ALL_USES_DONE[@]}
fi

if [ "${GENERATE_PACKAGE}" == "1" ]; then
    echo -n "Creating archive "

    declare -A sorted_UPP_ALL_USES_DONE=$(printf "%s\n" "${UPP_ALL_USES_DONE[@]}" | sort -u);

    package_src_name_archive=$(basename ${PROJECT_NAME}).tar.bz2
    package_src_name_archive_list="package_archive_list.txt"

    echo "CMakeLists.txt" > ${package_src_name_archive_list}

    find $(dirname ${PROJECT_NAME}) -name '*' -type f >> ${package_src_name_archive_list}

    echo "${UPP_SRC_DIR}/uppconfig.h" >> ${package_src_name_archive_list}
    echo "${UPP_SRC_DIR}/guiplatform.h" >> ${package_src_name_archive_list}

    for pkg_name in ${sorted_UPP_ALL_USES_DONE[@]}; do
        find ${UPP_SRC_DIR}/${pkg_name} -name '*' -type f >> ${package_src_name_archive_list}
    done

    tar -c -j -f ${package_src_name_archive} -T ${package_src_name_archive_list}
    rm ${package_src_name_archive_list}

    echo "... DONE"
fi

