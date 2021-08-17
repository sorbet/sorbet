# frozen_string_literal: true
# typed: strict

class Package::Subpackage::SubpackageClass
  extend T::Sig

  sig {returns(Package::PackageClass)}
  def self.package_class
    return Package::PackageClass.new()
  end
end
