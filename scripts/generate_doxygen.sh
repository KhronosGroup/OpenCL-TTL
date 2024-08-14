#!/bin/bash

# generate_doxygen.h
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

set -e

# always run from WORKSPACE
SCRIPTS_DIR=$(dirname "$0")
WORKSPACE="${WORKSPACE:-$(realpath $SCRIPTS_DIR/..)}"

if [[ "${!#}" == "-h" ]]; then
	echo "Usage: $0"
	exit -1
fi

pushd $WORKSPACE

rm -rf gh-pages/html
rm -rf gh-pages/xml
rm -rf gh-pages/*.pu


RECREATION_DIRECTORY=`mktemp -d XXXXXXX_TTL`
cd ${RECREATION_DIRECTORY}

# One we create a single preprocessed for opencl
$WORKSPACE/scripts/preprocess.sh -r -o TTL_doxygen.h

# Two copy everything into the RECRECTION directory.
# using rsync to exclude the RECREATION_DIRECTORY or it becomes recursive.
rsync -av --progress $WORKSPACE/ $WORKSPACE/${RECREATION_DIRECTORY} --exclude ${RECREATION_DIRECTORY}

# Three recreate the original files from the pre-processed directory
# this will overwrite the file copy in step 2 when applicable
$WORKSPACE/scripts/split_doxgen.py TTL_doxygen.h
rm TTL_doxygen.h

cd $WORKSPACE

# Seems to be a bug that plantuml leaves inline_umlgraph_cache_all.pu behind

# Run doxygen on the reconstituted doxygen
(cat $WORKSPACE/scripts/doxyfile.in; echo "INPUT=${RECREATION_DIRECTORY}") | doxygen -

rm -rf ${RECREATION_DIRECTORY}

popd
