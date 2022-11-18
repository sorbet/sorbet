#!/usr/bin/env bash

set -euo pipefail

# The way we regenerate document symbols is different from the way that we test
# document symbols are correct, so this "test" is just a forcing function to
# make sure that the print_document_symbols target is built in CI.
test -f test/print_document_symbols
