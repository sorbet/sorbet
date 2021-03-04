tmp=$(mktemp -d)
cp test/cli/autocorrect-payload/*.rb "$tmp"
main/sorbet --silence-dev-message -a "$tmp"/*.rb 2>/dev/null
cat "$tmp"/*.rb
