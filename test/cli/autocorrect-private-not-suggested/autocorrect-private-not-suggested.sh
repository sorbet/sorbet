# verifies that the file is unchanged when suggestions are applied
# i.e. it does not replace the unknown method with a private one
tmp=$(mktemp -d)
cp test/cli/autocorrect-private-not-suggested/*.rb "$tmp"
main/sorbet --silence-dev-message -a "$tmp"/*.rb 2>/dev/null
cat "$tmp"/*.rb
rm -rf "$tmp"
