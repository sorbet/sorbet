#!/bin/bash

set -euo pipefail
main/sorbet --silence-dev-message -e 'class Foo; end' --progress
