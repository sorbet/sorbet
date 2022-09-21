#!/bin/bash

set -euo pipefail

# TODO: We probably want to move this into https://github.com/sorbet/sorbet-build-image eventually

apt-get update
apt-get install -y libxshmfence-dev libnss3-dev libatk1.0-0 libatk-bridge2.0-0 \
 libdrm2 xvfb libgdk-pixbuf2.0-0 libgtk-3-0 libgbm1	libasound2

source .buildkite/tools/with_backoff.sh

pushd vscode_extension

# Sometimes requests to https://registry.yarnpkg.com hit transient errors.
with_backoff yarn

xvfb-run -a yarn test

popd
