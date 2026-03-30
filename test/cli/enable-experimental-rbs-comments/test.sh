#!/bin/bash

set -euo pipefail

rbs_example=$(cat <<'EOF'
#: (Object) -> Integer
def demo(x)
  x #: as String
end
EOF
)

common_args=(
  -e "$rbs_example"
  --silence-dev-message
  --quiet
  --no-error-count
  --print=desugar-tree
  --stop-after=desugarer
)

echo "==== test correct usage ===="
main/sorbet "${common_args[@]}" --enable-experimental-rbs-comments --parser=prism 2>&1

echo "==== test deprecated use with unset parser ===="
main/sorbet "${common_args[@]}" --enable-experimental-rbs-comments 2>&1

echo "==== test deprecated use with original parser ===="
main/sorbet "${common_args[@]}" --enable-experimental-rbs-comments --parser=original 2>&1

