tmp="$(mktemp -d)"
infile="test/cli/autocorrect-helpers/autocorrect-helpers.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a autocorrect-helpers.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat autocorrect-helpers.rb

rm autocorrect-helpers.rb
rm "$tmp"
