cd test/cli/packager-layers || exit 1

# --packager-layers flag with no argument with --sorbet-packages
../../../main/sorbet --silence-dev-message --sorbet-packages --packager-layers . 2>&1

# No --packager-layers flag with no --sorbet-packages
../../../main/sorbet --silence-dev-message . 2>&1

# --packager-layers flag with --sorbet-packages
../../../main/sorbet --silence-dev-message --sorbet-packages --packager-layers=a,b,c . 2>&1

# --packager-layers flag with no --sorbet-packages
../../../main/sorbet --silence-dev-message --packager-layers=a,b . 2>&1

# No --packager-layers flag with --sorbet-packages
../../../main/sorbet --silence-dev-message --sorbet-packages . 2>&1

# --packager-layers flag with underscored layer names
../../../main/sorbet --silence-dev-message --sorbet-packages --packager-layers=test_infra,test_helper,test_unit . 2>&1

# --packager-layers flag with a layer name that is not alphanumeric or underscores
../../../main/sorbet --silence-dev-message --sorbet-packages --packager-layers=test_infra,test-unit . 2>&1
