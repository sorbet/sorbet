# typed: strict

class PackageClass
    extend T::Sig

    sig {returns(Package::Subpackage::SubpackageClass)}
    def self.subpkg_class
        Package::Subpackage::SubpackageClass.new()
    end
end
