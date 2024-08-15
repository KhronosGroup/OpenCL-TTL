#!/bin/sh

set -x
set -e

# $1 is the directory where doxygen script should be executed
if [ ! -d $1 ]; then
  echo "Path $1 could not be found!"
  exit 1
fi

cd $1

# Add the packages required by scripts/generate_doxygen.sh
apk add bash doxygen graphviz ttf-freefont coreutils clang clang-extra-tools rsync python3

# execute doxygen
bash scripts/generate_doxygen.sh