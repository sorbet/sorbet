cd test/cli/wip-package || exit 1

../../../main/sorbet --silence-dev-message \
  --max-threads=0 \
  --stripe-packages . 2>&1
