#!/bin/bash

set -euo pipefail

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac


if [[ "linux" == "$platform" ]]; then
  CONFIG_OPTS="--config=buildfarm-sanitized-linux"
elif [[ "mac" == "$platform" ]]; then
  CONFIG_OPTS="--config=buildfarm-sanitized-mac"
fi

export JOB_NAME=test-static-sanitized
source .buildkite/tools/setup-bazel.sh

echo will run with $CONFIG_OPTS

err=0

# NOTE: running ruby/gem/srb testing without the sanitized flags
./bazel test @ruby_2_6_3//... @ruby_2_4_3//... @gems//... //gems/sorbet/test/snapshot \
  --config=buildfarm-ruby || err=$?

./bazel test //... $CONFIG_OPTS --test_summary=terse || err=$?

echo "--- uploading test results"

rm -rf _tmp_
mkdir -p _tmp_/log/junit/

# TODO: does this query omit the snapshot tests?
./bazel query 'tests(//...) except attr("tags", "manual", //...)' | while read -r line; do
    path="${line/://}"
    path="${path#//}"
    cp "bazel-testlogs/$path/test.xml" _tmp_/log/junit/"${path//\//_}-${BUILDKITE_JOB_ID}.xml"
done

annotation_dir="$(mktemp -d "junit-annotate-plugin-annotation-tmp.XXXXXXXXXX")"
annotation_path="${annotation_dir}/annotation.md"

function cleanup {
  rm -rf "${annotation_dir}"
}

trap cleanup EXIT


.buildkite/tools/annotate.rb _tmp_/log/junit > "$annotation_path"

cat "$annotation_path"

if grep -q "<details>" "$annotation_path"; then
  echo "--- :buildkite: Creating annotation"
  # shellcheck disable=SC2002
  cat "$annotation_path" | buildkite-agent annotate --context junit-${platform} --style error --append
fi

if [ "$err" -ne 0 ]; then
  if [[ "linux" == "$platform" ]] && [[ "${BUILDKITE_BRANCH}" != "" ]] && [[ "${BUILDKITE_BRANCH}" != "master" ]]; then
    # see if we can fix it!
    ./tools/scripts/update_exp_files.sh
    dirty=
    git diff-index --quiet HEAD -- || dirty=1
    if [ "$dirty" != "" ]; then
      git commit -a "Update exp files"
      git remote add pr_source "${BUILDKITE_PULL_REQUEST_REPO}"
      if [[ "${BUILDKITE_REPO}" == "git@github.com:stripe/sorbet.git" ]]; then
        branch_name="${BUILDKITE_BRANCH}"
      else
        branch_name="$(echo "${BUILDKITE_BRANCH}" | cut -d ":" -f 2)"
      fi
      git push pr_source "HEAD:${branch_name}"
    fi
  fi
  exit "$err"
fi
