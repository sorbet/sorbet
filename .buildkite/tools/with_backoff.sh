#!/usr/bin/env bash

set -euo pipefail

echo "Loading with_backoff helper"

# https://stackoverflow.com/a/8351489
with_backoff() {
  local attempts=8
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
