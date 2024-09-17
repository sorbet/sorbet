# Ensures that Sorbet doesn't complain about the package having behavior defined in multiple files in Stripe mode.
main/sorbet --silence-dev-message --uniquely-defined-behavior --stripe-packages --dir test/cli/package-multiple-files-behavior 2>&1
