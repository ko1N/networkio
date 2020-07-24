#!/bin/bash

set -e

HOOK_NAMES="pre_commit"
GIT_ROOT=$(git rev-parse --show-toplevel)
DEFAULT_HOOKS_DIR=${GIT_ROOT}/.git/hooks
CUSTOM_HOOKS_DIR=${GIT_ROOT}/git/hooks

for HOOK in ${HOOK_NAMES}; do
  # if custom hook exists and is executable then create symlink
  if [ -e ${CUSTOM_HOOKS_DIR}/${HOOK}.sh ] && [ -x ${CUSTOM_HOOKS_DIR}/${HOOK}.sh ]
  then
    echo "hook enabled: ${CUSTOM_HOOKS_DIR}/${HOOK}.sh"
    ln -s -f ../../git/hooks/${HOOK}.sh ${DEFAULT_HOOKS_DIR}/${HOOK}
  fi
done
