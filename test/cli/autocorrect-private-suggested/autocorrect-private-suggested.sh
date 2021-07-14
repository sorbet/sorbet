# verifies that the private method is suggested as a replacement
tmp="$(mktemp -d)"
infile="test/cli/autocorrect-private-suggested/autocorrect-private-suggested.rb"
cp "$infile" "$tmp"

old_pwd="$(pwd)"
cd "$tmp" || exit 1

"$old_pwd/main/sorbet" --silence-dev-message -a autocorrect-private-suggested.rb 2>&1
