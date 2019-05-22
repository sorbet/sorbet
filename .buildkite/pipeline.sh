#!/bin/bash

set -euo pipefail

whitelisted=0

if [[ "$BUILDKITE_PULL_REQUEST" == "false" ]]; then
  # whitelist commits that are triggered in branch builds of github.com/stripe/sorbet
  whitelisted=1
fi

if [[ "$BUILDKITE_PULL_REQUEST_REPO" == "git://github.com/stripe/sorbet.git" ]]; then
  # whitelist folks with write access to github.com/stripe/sorbet
  whitelisted=1
fi

pipeline=$(mktemp)
cp .buildkite/pipeline.yaml "$pipeline"

if [[ -n "${CLEAN_BUILD-}" ]]; then
    tmpfile=$(mktemp)
    sed -e '/Run optional code coverage/{N;d;}' "$pipeline" > "$tmpfile"
    mv "$tmpfile" "$pipeline"
fi

if [[ "${whitelisted}" -ne 1 ]] ; then
    tmpfile=$(mktemp)
   (echo -e "steps:\\n  - block: \":key: Needs contributor approval!\"\\n  - wait: ~\\n";
    grep -v "steps:" "$pipeline" ) > "$tmpfile"
    mv "$tmpfile" "$pipeline"
fi

buildkite-agent pipeline upload < "$pipeline"
