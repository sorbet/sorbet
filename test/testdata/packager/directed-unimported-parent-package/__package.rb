# typed: strict
# enable-packager: true
# enable-package-directed: true
# stratum: 2

class Parent < PackageSpec
  import Parent::Utils::Child
end
