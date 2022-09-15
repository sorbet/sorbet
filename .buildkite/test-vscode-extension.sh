#!/bin/bash

set -euo pipefail

# TODO: We probably want to move this into https://github.com/sorbet/sorbet-build-image eventually

apt-get update
apt-get install -y libxshmfence-dev libnss3-dev libatk1.0-0 libatk-bridge2.0-0 \
 libdrm2 xvfb libgdk-pixbuf2.0-0 libgtk-3-0 libgbm1	libasound2

pushd vscode_extension

# https://stackoverflow.com/a/8351489
with_backoff() {
  local attempts=5
  local timeout=1 # doubles each failure

  local attempt=0
  while true; do
    attempt=$(( attempt + 1 ))
    echo "Attempt $attempt"
    if "$@"; then
      return 0
    fi

    if (( attempt >= attempts )); then
      echo "'$1' failed $attempts times. Quitting." 1>&2
      exit 1
    fi

    echo "'$1' failed. Retrying in ${timeout}s..." 1>&2
    sleep $timeout
    timeout=$(( timeout * 2 ))
  done
}

# Sometimes requests to https://registry.yarnpkg.com hit transient errors.
with_backoff yarn

xvfb-run -a yarn test

popd
