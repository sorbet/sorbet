# typed: strict
# enable-packager: true
# enable-package-directed: true
# disable-fast-path: true

# stratum: 2

class Child < PackageSpec
  import Parent
end
