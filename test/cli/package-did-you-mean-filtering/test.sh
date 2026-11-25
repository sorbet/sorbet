cd test/cli/package-did-you-mean-filtering || exit 1

../../../main/sorbet --silence-dev-message --sorbet-packages --max-threads=0 . 2>&1
