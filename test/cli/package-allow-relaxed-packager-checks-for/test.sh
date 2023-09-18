cd test/cli/package-allow-relaxed-packager-checks-for || exit 1

../../../main/sorbet --silence-dev-message --stripe-packages --allow-relaxed-packager-checks-for=Project::Root --max-threads=0 . 2>&1


