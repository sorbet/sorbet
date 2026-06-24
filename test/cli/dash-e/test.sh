#!/bin/bash

set -euo pipefail
main/sorbet --silence-dev-message --censor-for-snapshot-tests -e 'class Foo; end' -p symbol-table 2>&1
main/sorbet --silence-dev-message --censor-for-snapshot-tests -e $'# typed: strong\ndef foo; end' 2>&1
