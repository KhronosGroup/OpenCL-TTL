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
THIS_DIR=$(dirname "$0")
WORKSPACE="${WORKSPACE:-$(realpath $THIS_DIR/..)}"

if [[ "${!#}" == "-h" ]]; then
	echo "Usage: $0"
	exit -1
fi

pushd $WORKSPACE
rm -rf html
rm -rf xml

# Seems to be a bug that plantuml leaves inline_umlgraph_cache_all.pu behind
rm -f inline_umlgraph_cache_all.pu

doxygen $WORKSPACE/scripts/doxyfile.in

popd
