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
SCRIPTS_DIR=$(dirname "$0")
WORKSPACE="${WORKSPACE:-$(realpath $SCRIPTS_DIR/..)}"

OUTPUT_FILE=/dev/stdout
TTL_TARGET=opencl
REMOVE_LINE_NUMBERS="y"

while getopts ":o:t:r" opt; do
  case $opt in
    o) OUTPUT_FILE="$PWD"/"$OPTARG"
    ;;
    t) TTL_TARGET="$OPTARG"
    ;;
    r) REMOVE_LINE_NUMBERS="n"
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

pushd ${WORKSPACE}

WORKING_FILE=`mktemp XXXXXXX.h`

clang -DTTL_TARGET=${TTL_TARGET} -DTTL_COPY_3D -E -CC scripts/preprocess.c -o ${WORKING_FILE}

if [[ "${REMOVE_LINE_NUMBERS}" == "y" ]]; then
	sed -i '/^# [0-9]*/d' ${WORKING_FILE}
else
	sed -i 's/scripts\/..\///g' ${WORKING_FILE}
fi

sed -i -e '1,/TTL_START_MARKER/d' ${WORKING_FILE}

##################
# Below is a bit of a work in progress to try and sed our way to code that produces
# the best output when clang-format runs
##################

# Clang format to get an evenish starting point
clang-format -i ${WORKING_FILE}

# We end up with trailing \ because of the macros, these mean nothing post processing so replace them.
sed -i 's/\\$//g' ${WORKING_FILE}

# When an end comment marker is not the end of the line then add a new line
sed -i 's/\*\/[^%]/\*\/\n/g' ${WORKING_FILE}

# Add some blank lines before comments, clang-format then evens it all up.
# We look for three types /*\n /**\n because we want to avoid /**<
sed -i 's/\/\*[\n ]/\n\n\n\n\/\*/g' ${WORKING_FILE}
sed -i 's/\/\*\*[\n ]/\n\n\n\n\/\*/g' ${WORKING_FILE}

# Now get clang to make it pretty!
clang-format -i ${WORKING_FILE}

echo '#pragma once' >${OUTPUT_FILE}
echo >>${OUTPUT_FILE}
echo '#include "TTL_macros.h"' >>${OUTPUT_FILE}
echo >>${OUTPUT_FILE}
cat ${WORKING_FILE} >>${OUTPUT_FILE}

rm ${WORKING_FILE}

popd