#!/usr/bin/env bash

set -euo pipefail

diff -u "{expected_output}" "{test_output}"
