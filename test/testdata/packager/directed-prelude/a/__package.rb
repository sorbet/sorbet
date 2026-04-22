# typed: strict
# enable-packager: true
# enable-package-directed: true

# Necessarily at 0 because we're a prelude package

# stratum: 0

class A < PackageSpec
  prelude_package
end
