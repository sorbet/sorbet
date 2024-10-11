cd test/cli/packager-layer || exit 1

# No --packager-layer flag with --stripe-packages
../../../main/sorbet --silence-dev-message --stripe-packages . 2>&1


# No --packager-layer flag with no --stripe-packages
../../../main/sorbet --silence-dev-message . 2>&1

# --packager-layer flag with --stripe-packages
../../../main/sorbet --silence-dev-message --stripe-packages --packager-layer=a,b,c . 2>&1

# --packager-layer flag with no --stripe-packages
../../../main/sorbet --silence-dev-message --packager-layer=a,b . 2>&1
