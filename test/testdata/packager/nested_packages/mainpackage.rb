# frozen_string_literal: true
# typed: strict

class Package::PackageClass
    extend T::Sig

    sig {returns(Package::Subpackage::SubpackageClass)}
    def self.subpkg_class
        Package::Subpackage::SubpackageClass.new()
    end
end
