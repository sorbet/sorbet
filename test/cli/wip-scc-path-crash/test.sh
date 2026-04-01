cd test/cli/wip-scc-path-crash || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --packager-layers --max-threads=0 . 2>&1
