# frozen_string_literal: true

# typed: strict
# enable-packager: true

class Package < PackageSpec
  import Package::Subpackage
  export Package::PackageClass
  export Package::InnerClass
end
