#!/bin/sh

# This script imports test cases from the whitequark/parser repo.
# It should be run manually, and it's not run by any automated test suite.
# When executed on fresh master it should produce zero changes.
#
# To bump version of the whitequark/parser that sorbet/parser mirrors
# change "REF" variable and re-run the script. Then update the code to match tests.
#
# The script runs whitequark/parser tests only for a version of Ruby
# specified in "TARGET_RUBY_VERSION" variable, bump it if you want to upgrade
# Sorbet's parser to the next version

set -euo pipefail
set -x

REF=fbc2d7b14b838845af35b3c4596ae61c3f696ad1
TARGET_RUBY_VERSION="3.2"

SCRIPT=$(realpath "$0")
ROOT="$(cd "$(dirname "$SCRIPT")/../.."; pwd)"

function cleanup() {
    rm -rf "$ROOT/tmp"
}
trap cleanup EXIT

IMPORT_FROM="$ROOT/tmp/whitequark_parser"
IMPORT_TO="$ROOT/test/whitequark"

# Clone whitequark/parser repo
cd "$ROOT"
rm -rf tmp
mkdir -p tmp
cd tmp
git clone git@github.com:whitequark/parser.git whitequark_parser
cd whitequark_parser
git reset --hard $REF

# Install dependencies
bundle install --path ./.bundle

# Generate grammar files
ragel -F1 -R lib/parser/lexer.rl -o lib/parser/lexer.rb
bundle exec racc --superclass=Parser::Base lib/parser/ruby18.y -o lib/parser/ruby18.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby19.y -o lib/parser/ruby19.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby20.y -o lib/parser/ruby20.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby21.y -o lib/parser/ruby21.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby22.y -o lib/parser/ruby22.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby23.y -o lib/parser/ruby23.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby24.y -o lib/parser/ruby24.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby25.y -o lib/parser/ruby25.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby26.y -o lib/parser/ruby26.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby27.y -o lib/parser/ruby27.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby30.y -o lib/parser/ruby30.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby31.y -o lib/parser/ruby31.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/ruby32.y -o lib/parser/ruby32.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/macruby.y -o lib/parser/macruby.rb --no-line-convert
bundle exec racc --superclass=Parser::Base lib/parser/rubymotion.y -o lib/parser/rubymotion.rb --no-line-convert

# Drop and re-create a directory with tests
rm -rf "$IMPORT_TO"
mkdir -p "$IMPORT_TO"

# Run importer
TARGET_RUBY_VERSION="$TARGET_RUBY_VERSION" \
    IMPORT_FROM="$IMPORT_FROM" \
    IMPORT_TO="$IMPORT_TO" \
    ruby "$ROOT/tools/scripts/import_whitequark.rb"
