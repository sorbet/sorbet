# Ensures that Sorbet doesn't complain about the package having behavior defined in multiple files in Stripe mode.
main/sorbet --silence-dev-message --stripe-mode --stripe-packages --dir test/cli/package-multiple-files-stripe-mode 2>&1
