tmp="$(mktemp -d)"
infile="test/cli/autocorrect-case-t-boolean/autocorrect-case-t-boolean.rb"
cp "$infile" "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

"$cwd/main/sorbet" --silence-dev-message -a autocorrect-case-t-boolean.rb 2>&1

echo
echo --------------------------------------------------------------------------
echo

# Also cat the file, to make sure that 'extend' is only added once per class.
cat autocorrect-case-t-boolean.rb

rm autocorrect-case-t-boolean.rb
rm "$tmp"
