for ext in rb rbi; do
  tmp="$(mktemp).$ext"
  pre="test/cli/autocorrect-remove-body/pre.$ext"
  post="test/cli/autocorrect-remove-body/post.$ext"
  cp "$pre" "$tmp"
  main/sorbet --silence-dev-message -a "$tmp"
  diff "$post" "$tmp"
  rm "$tmp"
done
