#!/bin/bash

set -exuo pipefail

whitelisted=0

if [[ "${whitelisted}" -ne 1 ]] ; then
   (echo -e "steps:\n  - block: \":key: Needs contributor approval!\"\n  - wait: ~\n";
   cat .buildkite/pipeline.yaml | grep -v "steps:") | buildkite-agent pipeline upload
else
  buildkite-agent pipeline upload
fi
