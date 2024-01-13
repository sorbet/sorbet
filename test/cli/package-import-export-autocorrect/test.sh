tmp="$(mktemp -d)"
cp -R test/cli/package-import-export-autocorrect/* "$tmp"

main/sorbet --silence-dev-message --stripe-packages --max-threads=0 --autocorrect "$tmp" 2>&1

echo "===================="

cat "$tmp/b/__package.rb"
rm -rf "$tmp"
