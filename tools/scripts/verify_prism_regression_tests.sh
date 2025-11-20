#!/bin/bash
set -euo pipefail

echo "Building Sorbet..."
./bazel build //main:sorbet --config=dbg

echo "Verifying parse trees and desugar trees..."

# Files to skip (known behavior mismatches)
skip_files=(
  "call_kw_nil_args"
  "call_block_param_and_forwarding"
)

mismatched_parse_tree_files=()
mismatched_desugar_tree_files=()

# Iterate through Ruby files in test/prism_regression
for file in test/prism_regression/*.rb; do
  file_name=$(basename "$file" .rb)

  if [[ " ${skip_files[@]} " =~ " ${file_name} " ]]; then
    echo "⏭️  ${file_name}.rb (skipped)"
    continue
  fi

  # Generate parse tree
  set +e # Disable exit on error for the next command
  ./bazel-bin/main/sorbet --stop-after=desugarer --print=parse-tree:temp_parse_tree.txt --print=desugar-tree-raw:temp_desugar_tree.txt "$file" 2>/dev/null
  set -e # Re-enable exit on error

  parse_tree_match=true
  desugar_tree_match=true

  # Compare parse tree with existing parse tree
  if ! diff -q "temp_parse_tree.txt" "test/prism_regression/${file_name}.rb.parse-tree.exp" > /dev/null 2>&1; then
    parse_tree_match=false
    mismatched_parse_tree_files+=($file_name)
  fi

  # Compare desugar tree with existing desugar tree
  if ! diff -q "temp_desugar_tree.txt" "test/prism_regression/${file_name}.rb.desugar-tree-raw.exp" > /dev/null 2>&1; then
    desugar_tree_match=false
    mismatched_desugar_tree_files+=($file_name)
  fi

  # Print status
  if [ "$parse_tree_match" = true ] && [ "$desugar_tree_match" = true ]; then
    echo "✅ ${file_name}.rb"
  else
    status="❌ ${file_name}.rb"
    if [ "$parse_tree_match" = false ]; then
      status="${status} (parse-tree)"
    fi
    if [ "$desugar_tree_match" = false ]; then
      status="${status} (desugar-tree-raw)"
    fi
    echo "$status"
  fi
done

# Clean up temporary files
rm temp_parse_tree.txt temp_desugar_tree.txt

if [ ${#mismatched_parse_tree_files[@]} -gt 0 ] || [ ${#mismatched_desugar_tree_files[@]} -gt 0 ]; then
  echo ""
  echo "Some of your test files are out of date. Run the following commands to regenerate them:"
  echo ""

  for file in "${mismatched_parse_tree_files[@]}"; do
    echo "bazel-bin/main/sorbet --stop-after=parser --print=parse-tree test/prism_regression/${file}.rb > test/prism_regression/${file}.rb.parse-tree.exp"
  done

  for file in "${mismatched_desugar_tree_files[@]}"; do
    echo "bazel-bin/main/sorbet --stop-after=desugarer --print=desugar-tree-raw test/prism_regression/${file}.rb > test/prism_regression/${file}.rb.desugar-tree-raw.exp"
  done

  exit 1
else
  echo ""
  echo "All parse trees and desugar trees verified successfully!"
fi
