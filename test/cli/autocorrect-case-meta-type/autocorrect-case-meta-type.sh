tmp="$(mktemp -d)"
infile="test/cli/autocorrect-case-meta-type/autocorrect-case-meta-type.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a autocorrect-case-meta-type.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat autocorrect-case-meta-type.rb

rm autocorrect-case-meta-type.rb
rm "$tmp"
