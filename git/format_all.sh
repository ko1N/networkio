#!/bin/bash

pushd ../

set -e

FILES=$(git ls-files | grep -E "\.(c|h|cpp|hpp|cc|hh)$")
for FILE in ${FILES}; do
    clang-format -style=file -i ${FILE}
done

SUBFILES=$(git submodule foreach --recursive git ls-files | grep -E "\.(c|h|cpp|hpp|cc|hh)$")
for SUBFILE in ${SUBFILES}; do
    clang-format -style=file -i ${SUBFILE}
done

popd

exit 0
