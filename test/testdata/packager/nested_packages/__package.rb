# typed: strict
# enable-packager: true

class Package < PackageSpec
  import Package::Subpackage
  export PackageClass
end
