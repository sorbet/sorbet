cwd="$(pwd)"
tmp="$(mktemp -d)"
cd test/cli/package-double-import-autocorrect || exit 1
for file in $(find . -name '*.rb' | sort); do
    mkdir -p "$tmp/$(dirname "$file")"
    cp "$file" "$tmp/$file"
done
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message --stripe-packages --max-threads=0 --autocorrect . 2>&1

echo
echo "===================="
echo

cat same-file/__package.rb

echo
echo "===================="
echo

cat different-files/__package.rb
