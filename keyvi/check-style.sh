#!/bin/bash

clang-format -version

commit_range="${GITHUB_BASE_REF}...${GITHUB_SHA}"

if [ -n "${commit_range}" ]; then
commit_range="upstream/master...HEAD"
fi

infiles=`git diff --name-only --diff-filter=ACMRT $(echo ${commit_range} | sed 's/\.//') | grep -v "3rdparty" | grep -E "\.(cpp|h)$"`

clang_format_files=()
cpplint_files=()

for file in ${infiles}; do
  fqfile=`git rev-parse --show-toplevel`/${file}
  echo "Checking: ${file}"
  if ! cmp -s ${fqfile} <(clang-format ${fqfile}); then
    clang_format_files+=("${file}")
  fi
  if ! cpplint --quiet ${fqfile}; then
    cpplint_files+=("${file}")
  fi
done

rc=0

if [ -n "${clang_format_files}" ]; then
echo "Format error (clang-format) within the following files:"
printf "%s\n" "${clang_format_files[@]}"
rc=1
fi


if [ -n "${cpplint_files}" ]; then
echo "Cpplint error within the following files:"
printf "%s\n" "${cpplint_files[@]}"
rc=1
fi

exit ${rc}
