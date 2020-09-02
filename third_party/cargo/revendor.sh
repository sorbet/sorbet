#!/bin/bash
set -euxo pipefail
rm -rf vendor
cargo generate-lockfile
cargo vendor --versioned-dirs --locked
cargo raze
