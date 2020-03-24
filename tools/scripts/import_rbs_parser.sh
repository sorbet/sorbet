#!/bin/sh

# This script imports the Shopify/rbs_parser repo under third party.
#
# It should be run manually, and it's not run by any automated test suite.
# When executed on fresh master it should produce zero changes.

set -euo pipefail

SCRIPT=$(realpath "$0")
ROOT="$(cd "$(dirname "$SCRIPT")/../.."; pwd)"

# Clone whitequark/parser repo
cd "$ROOT"
rm -rf tmp
mkdir -p tmp
git clone git@github.com:Shopify/rbs_parser.git tmp/rbs_parser

rm -rf third_party/rbs_parser/src
cp -R tmp/rbs_parser/src third_party/rbs_parser

rm -rf tmp
