# typed: strict
# enable-packager: true
# enable-package-directed: true

class A < PackageSpec
  import A::B
  export A::Thing
end
