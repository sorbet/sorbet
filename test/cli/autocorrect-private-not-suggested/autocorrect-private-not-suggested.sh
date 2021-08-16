tmp="$(mktemp -d)"
cp test/cli/autocorrect-private-not-suggested/*.rb "$tmp"

cwd="$(pwd)"
cd "$tmp" || exit 1

if "$cwd/main/sorbet" --silence-dev-message -a autocorrect-private-not-suggested.rb 2>&1; then
  echo "Expected to fail!"
  exit 1
fi

echo
echo --------------------------------------------------------------------------
echo

# verifies that the file is unchanged when suggestions are applied
# i.e. it does not replace the unknown method with a private one
cat autocorrect-private-not-suggested.rb

rm autocorrect-private-not-suggested.rb
rm "$tmp"