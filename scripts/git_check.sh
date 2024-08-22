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

# TTL pre-commit hooks

# Use relative paths below, this requires no symlink an not setup.
SCRIPTS_DIR=`dirname "$0"`/git-hooks

check_format=true
check_copyright=true

while getopts fch flag
do
    case "${flag}" in
		f) check_format=false;;
		c) check_copyright=false;;
		h) show_help=true;;
    esac
done

if [ "$show_help" = true ]; then
	echo "Usage: git_check.sh [-f] [-c] [-h]"
	echo "  -f  Skip clang-format check"
	echo "  -c  Skip copyright check"
	echo "  -h  Show this help message"
	exit 0
fi

if [ "$check_format" = true ]; then
	# Apply clang-format to all changed files
	# the configuration is in the root of the
	# repo in the .clang-format file
	$SCRIPTS_DIR/clang-format HEAD^
fi

if [ "$check_copyright" = true ]; then
	# Check Copyrights are correct
	$SCRIPTS_DIR/copyright-check HEAD^
fi

git diff --exit-code

result=$?

if [ $result == 0 ]; then
   echo "Checked if git triggers ran - OK"
else
   echo "Checked if git triggers ran - FAIL"
fi

exit $result
