#!/bin/bash

set -euo pipefail

trap 'rm -f using_namespace.output' EXIT

if git grep --line '^\s*using namespace std;' '*.h' > using_namespace.output; then
  echo "Please don't use \`using namespace std;\` in header files."
  echo ""
  echo "Header files should fully-qualify standard library names, but feel free"
  echo "to use \`using namespace std;\` in *.cc files."
  echo ""
  echo "\`\`\`"
  cat using_namespace.output
  echo "\`\`\`"

  exit 1
fi
