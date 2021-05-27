# frozen_string_literal: true
# typed: strict

class Package::Subpackage < PackageSpec
  import Package
  export Package::Subpackage::SubpackageClass
end
