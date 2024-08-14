#!/bin/bash -e

# git_check.sh
#
# Copyright (c) 2023 Mobileye
#
# Licensed under the Apache License, Version 2.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Precompile TTL and output to stdout

# Use relative paths below, this requires no symlink an not setup.
SCRIPTS_DIR=`dirname "$0"`
WORKING_FILE=`mktemp XXXXXXX.h`

OUTPUT_FILE=/dev/stdout
TTL_TARGET=opencl

while getopts ":o:t:" opt; do
  case $opt in
    o) OUTPUT_FILE="$OPTARG"
    ;;
    t) TTL_TARGET="$OPTARG"
    ;;
    \?) echo "Invalid option -$OPTARG" >&2
    exit 1
    ;;
  esac

  case $OPTARG in
    -*) echo "Option $opt needs a valid argument"
    exit 1
    ;;
  esac
done

clang -DTTL_TARGET=${TTL_TARGET} -DTTL_COPY_3D -I ${SCRIPTS_DIR}/.. -E -CC ${SCRIPTS_DIR}/preprocess.c -o ${WORKING_FILE}
sed -i '/^# [0-9]*/d' ${WORKING_FILE}
sed -i -e '1,/TTL_START_MARKER/d' ${WORKING_FILE}
clang-format -i ${WORKING_FILE}

echo '#pragma once' >${OUTPUT_FILE}
echo >>${OUTPUT_FILE}
echo '#include "TTL_macros.h"' >>${OUTPUT_FILE}
echo >>${OUTPUT_FILE}
cat ${WORKING_FILE} >>${OUTPUT_FILE}

rm ${WORKING_FILE}