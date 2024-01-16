tmp="$(mktemp -d)"
cp -R test/cli/package-import-export-autocorrect/* "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message --stripe-packages --max-threads=0 --autocorrect . 2>&1

echo "===================="

cat b/__package.rb
