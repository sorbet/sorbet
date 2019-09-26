tmp=$(mktemp)
test/llvm/kaleidoscope 2> "$tmp"
diff "$tmp" test/llvm/kaleidoscope_test.out
