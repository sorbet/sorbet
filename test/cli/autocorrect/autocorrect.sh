tmp=$(mktemp)
file=test/cli/autocorrect/autocorrect.rb
cp "$file" "$tmp"
main/sorbet --silence-dev-message -a "$tmp"
diff "$file" "$tmp"
rm "$tmp"
