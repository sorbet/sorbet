# typed: strict
# enable-packager: true
# stratum: 2

class Parent < PackageSpec
  import Parent::Utils::Child
end
