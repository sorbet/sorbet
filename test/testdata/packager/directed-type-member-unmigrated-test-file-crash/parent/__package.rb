# typed: strict
# enable-packager: true
# enable-package-directed: true
# enable-test-packages: true

# stratum: 0

class Parent < PackageSpec
  export Parent::MyGeneric
end
