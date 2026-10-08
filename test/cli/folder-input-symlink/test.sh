#!/bin/bash

set -euo pipefail

sorbet="$(pwd)/main/sorbet"

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

cd "$tmp"
mkdir -p input/ignored references/lib references/excluded
cat >references/shared.rbi <<'RBI'
# typed: true
class SharedFromRBI
  extend T::Sig
  sig {returns(Integer)}
  def self.answer; end
end
RBI
touch references/lib/nested.rb references/lib/file-link.rb
touch references/excluded/ignored.rbi input/ignored/ignored.rb input/local.rb
cat >input/consumer.rb <<'RUBY'
# typed: true
T.let(SharedFromRBI.answer, Integer)
RUBY

cd input
ln -s ../references references
ln -s ../references/lib/file-link.rb file-link.rb
ln -s . self-loop
ln -s . ../references/self-loop

echo '------------------------------------------------------------------------'
# Directory symlinks are followed (keeping lexical paths), self-loops terminate,
# and ignore patterns match lexical routes, including inside the linked directory.
"$sorbet" --silence-dev-message --no-config -p file-table-json --no-stdlib --stop-after parser --dir . \
  --ignore=/ignored --ignore=/references/excluded

echo '------------------------------------------------------------------------'
# The linked RBI is used for typechecking.
"$sorbet" --silence-dev-message --censor-for-snapshot-tests --no-config --dir . \
  --ignore=/ignored --ignore=/references/excluded 2>&1

echo '------------------------------------------------------------------------'
cat >consumer.rb <<'RUBY'
# typed: true
T.let(SharedFromRBI.answer, String)
RUBY
if "$sorbet" --silence-dev-message --censor-for-snapshot-tests --no-config --dir . \
  --ignore=/ignored --ignore=/references/excluded 2>&1; then
  echo "Expected to fail!"
  exit 1
fi
