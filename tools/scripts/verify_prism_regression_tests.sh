#!/bin/bash
set -euo pipefail

echo "Building Sorbet..."
./bazel build //main:sorbet --config=dbg --define RUBY_PATH=$RUBY_ROOT

echo "Verifying parse trees..."

mismatched_files=()

# Iterate through Ruby files in test/prism_regression
for file in test/prism_regression/*.rb; do
  file_name=$(basename "$file" .rb)

  # Generate parse tree
  set +e # Disable exit on error for the next command
  ./bazel-bin/main/sorbet --stop-after=parser --print=parse-tree "$file" > temp_parse_tree.txt 2> /dev/null
  set -e # Re-enable exit on error

  # Compare with existing parse tree
  if diff -q "temp_parse_tree.txt" "test/prism_regression/${file_name}.parse-tree.exp" > /dev/null; then
    echo "✅ ${file_name}.rb"
  else
    echo "❌ ${file_name}.rb"
    mismatched_files+=($file_name)
  fi
done

# Clean up temporary file
rm temp_parse_tree.txt

if [ ${#mismatched_files[@]} -gt 0 ]; then
  echo ""
  echo "Some of your parse trees are out of date. Run the following commands to regenerate them:"
  echo ""

  for file in "${mismatched_files[@]}"; do
    echo "bazel-bin/main/sorbet --stop-after=parser --print=parse-tree test/prism_regression/${file}.rb > test/prism_regression/${file}.parse-tree.exp"
  done

  exit 1
else
  echo ""
  echo "All parse trees verified successfully!"
fi
