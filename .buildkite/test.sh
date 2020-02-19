#!/bin/bash

set -euo pipefail

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     platform="linux";;
    Darwin*)    platform="mac";;
    *)          exit 1
esac

if [[ "linux" == "$platform" ]]; then
    apt-get update
    apt-get install -yy libncurses5-dev libncursesw5-dev xxd
elif [[ "mac" == "$platform" ]]; then
    if ! [ -x "$(command -v wget)" ]; then
        brew install wget
    fi
fi

export JOB_NAME=test
source .buildkite/tools/setup-bazel.sh

err=0

./bazel test //... --config=dbg -c opt --test_summary=terse --test_output=errors || err=$?

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

  if [[ "linux" == "$platform" ]]; then
      wget -O - https://github.com/buildkite/terminal-to-html/releases/download/v3.2.0/terminal-to-html-3.2.0-linux-amd64.gz | gunzip -c > ./terminal-to-html
  elif [[ "mac" == "$platform" ]]; then
      wget -O - https://github.com/buildkite/terminal-to-html/releases/download/v3.2.0/terminal-to-html-3.2.0-darwin-amd64.gz | gunzip -c > ./terminal-to-html
  fi
  chmod u+x ./terminal-to-html

  # https://buildkite.com/docs/agent/v3/cli-annotate
  annotation_path_wrapped="${annotation_dir}/annotation_wrapped.md"
  (echo '<pre class="term"><code>'; ./terminal-to-html < "$annotation_path"; echo '</code></pre>') > "$annotation_path_wrapped"

  # shellcheck disable=SC2002
  cat "$annotation_path" | buildkite-agent annotate --context junit-${platform} --style error --append
fi

if [ "$err" -ne 0 ]; then
    exit "$err"
fi
