cd test/cli/package-layers-violations || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --stripe-packages-layers=lowest,middle,highest --max-threads=0 . 2>&1
