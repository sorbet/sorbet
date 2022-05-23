# shellcheck disable=SC2069

cd test/cli/autogen-constant-cache || exit 1

cp foo.rb example.rb

echo "Running autogen"
../../../main/sorbet --silence-dev-message -p autogen-msgpack --autogen-version=2 --stop-after=namer --skip-rewriter-passes --autogen-constant-cache-file cache.msgpack example.rb 2>&1 >/dev/null

echo
echo "Running autogen with no changes: should exit early"
../../../main/sorbet --silence-dev-message -p autogen-msgpack --autogen-version=2 --stop-after=namer --skip-rewriter-passes --autogen-constant-cache-file cache.msgpack foo.rb --autogen-changed-files example.rb 2>&1 >/dev/null

echo
echo "Running autogen with no changes (and './' path): should exit early"
../../../main/sorbet --silence-dev-message -p autogen-msgpack --autogen-version=2 --stop-after=namer --skip-rewriter-passes --autogen-constant-cache-file cache.msgpack foo.rb --autogen-changed-files ./example.rb 2>&1 >/dev/null

echo
echo "Running autogen with non-constant-related changes: should exit early"
cat >>example.rb <<EOF
def new_method
  puts "this method will not change the constant hash"
end
EOF
../../../main/sorbet --silence-dev-message -p autogen-msgpack --autogen-version=2 --stop-after=namer --skip-rewriter-passes --autogen-constant-cache-file cache.msgpack example.rb --autogen-changed-files example.rb 2>&1 >/dev/null

echo
echo "Running autogen with constant-related changes: should re-run"
cat >>example.rb <<EOF

module NewModule
end
EOF
../../../main/sorbet --silence-dev-message -p autogen-msgpack --autogen-version=2 --stop-after=namer --skip-rewriter-passes --autogen-constant-cache-file cache.msgpack example.rb --autogen-changed-files example.rb 2>&1 >/dev/null

echo
echo "Producing a new constant cache with './' paths should produce the same cache file:"
../../../main/sorbet --silence-dev-message -p autogen-msgpack --autogen-version=2 --stop-after=namer --skip-rewriter-passes --autogen-constant-cache-file abs-cache.msgpack ./example.rb >/dev/null 2>&1
if $(diff cache.msgpack abs-cache.msgpack >/dev/null); then
    echo "Same!"
else
    echo "Not same!"
fi
