tmp=$(mktemp)
file=test/cli/autocorrect/autocorrect.rb
cp "$file" "$tmp"
main/sorbet -a "$tmp"
diff "$file" "$tmp"
rm "$tmp"
