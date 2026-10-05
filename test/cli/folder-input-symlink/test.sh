#!/bin/bash

set -euo pipefail

sorbet="$(pwd)/main/sorbet"

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

mkdir -p "$tmp/input/ignored" "$tmp/references/lib" "$tmp/references/excluded"
cat >"$tmp/references/shared.rbi" <<'RBI'
# typed: true
class SharedFromRBI
  extend T::Sig
  sig {returns(Integer)}
  def self.answer; end
end
RBI
touch "$tmp/references/lib/nested.rb" "$tmp/references/lib/file-link.rb"
touch "$tmp/references/excluded/ignored.rbi" "$tmp/input/ignored/ignored.rb"
cat >"$tmp/input/consumer.rb" <<'RUBY'
# typed: true
T.let(SharedFromRBI.answer, Integer)
RUBY
touch "$tmp/input/local.rb"
ln -s ../references "$tmp/input/references"
ln -s ../references/lib/file-link.rb "$tmp/input/file-link.rb"
ln -s . "$tmp/input/self-loop"
ln -s . "$tmp/references/self-loop"

cd "$tmp/input"

file_table="$("$sorbet" --silence-dev-message --no-config -p file-table-json --no-stdlib --stop-after parser --dir . \
  --ignore=/ignored --ignore=/references/excluded)"
ruby -rjson -e '
  paths = JSON.parse(STDIN.read).fetch("files").map { |file| file.fetch("path") }
  expected = [
    "./consumer.rb", "./file-link.rb", "./local.rb", "./references/lib/file-link.rb",
    "./references/lib/nested.rb", "./references/shared.rbi",
  ]
  missing = expected - paths
  unexpected = paths - expected
  abort "missing lexical discovery paths: #{missing.join(", ")}" unless missing.empty?
  abort "unexpected logical discovery paths: #{unexpected.join(", ")}" unless unexpected.empty?
  puts "linked and file symlink paths discovered; ignored paths pruned"
' <<<"$file_table"

"$sorbet" --silence-dev-message --no-config --dir . --ignore=/ignored --ignore=/references/excluded
cat >consumer.rb <<'RUBY'
# typed: true
T.let(SharedFromRBI.answer, String)
RUBY
if typecheck_output="$("$sorbet" --silence-dev-message --no-config --dir . --ignore=/ignored \
  --ignore=/references/excluded 2>&1)"; then
  echo "expected the linked RBI's Integer return type to reject String" >&2
  exit 1
else
  [[ "$?" == 100 ]]
fi
ruby -e '
  errors = STDIN.read.scan(%r{https://srb.help/(\d+)}).flatten
  abort "unexpected typecheck errors: #{errors.inspect}" unless errors == ["7007"]
  puts "linked RBI consumer resolves to Integer; symlink cycles terminate"
' <<<"$typecheck_output"
