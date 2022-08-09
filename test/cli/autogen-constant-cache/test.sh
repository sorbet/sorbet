# shellcheck disable=SC2069

cd test/cli/autogen-constant-cache || exit 1

cp foo.rb example.rb

echo "Running autogen"
../../../main/sorbet --silence-dev-message -p autogen-msgpack --autogen-version=2 --stop-after=namer --autogen-constant-cache-file cache.msgpack example.rb 2>&1 >/dev/null

echo
echo "Running autogen with no changes: should exit early"
../../../main/sorbet --silence-dev-message -p autogen-msgpack --autogen-version=2 --stop-after=namer --autogen-constant-cache-file cache.msgpack foo.rb --autogen-changed-files example.rb 2>&1 >/dev/null

echo
echo "Running autogen with non-constant-related changes: should exit early"
cat >>example.rb <<EOF
def new_method
  puts "this method will not change the constant hash"
end
EOF
../../../main/sorbet --silence-dev-message -p autogen-msgpack --autogen-version=2 --stop-after=namer --autogen-constant-cache-file cache.msgpack example.rb --autogen-changed-files example.rb 2>&1 >/dev/null

echo
echo "Running autogen with constant-related changes: should re-run"
cat >>example.rb <<EOF

module NewModule
end
EOF
../../../main/sorbet --silence-dev-message -p autogen-msgpack --autogen-version=2 --stop-after=namer --autogen-constant-cache-file cache.msgpack example.rb --autogen-changed-files example.rb 2>&1 >/dev/null
