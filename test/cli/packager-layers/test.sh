cd test/cli/packager-layers || exit 1

# --packager-layers flag with no argument with --stripe-packages
../../../main/sorbet --silence-dev-message --stripe-packages --packager-layers . 2>&1

# No --packager-layers flag with no --stripe-packages
../../../main/sorbet --silence-dev-message . 2>&1

# --packager-layers flag with --stripe-packages
../../../main/sorbet --silence-dev-message --stripe-packages --packager-layers=a,b,c . 2>&1

# --packager-layers flag with no --stripe-packages
../../../main/sorbet --silence-dev-message --packager-layers=a,b . 2>&1

# No --packager-layers flag with --stripe-packages
../../../main/sorbet --silence-dev-message --stripe-packages . 2>&1
