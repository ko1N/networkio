#!/bin/bash

FILES=$(git diff --cached --name-only --diff-filter=ACMR | grep -E "\.(c|h|cpp|hpp|cc|hh)$")
set -e

for FILE in ${FILES}; do
    clang-format -style=file -i ${FILE}
    git add ${FILE}
done

exit 0

