# Ensures that Sorbet doesn't complain about the package having behavior defined in multiple files in "uniquely-defined-behavior" mode
main/sorbet --silence-dev-message --uniquely-defined-behavior --sorbet-packages --dir test/cli/package-multiple-files-behavior 2>&1
