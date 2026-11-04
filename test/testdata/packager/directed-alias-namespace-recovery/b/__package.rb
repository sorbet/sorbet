# typed: strict
# enable-packager: true
# enable-package-directed: true
# stratum: 0

class B < PackageSpec
  export B::Thing
  export B::Mixin
end
