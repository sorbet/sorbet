# typed: strict

# This package is in the root of the directory. It will be loaded in as file `./__package.rb`.

class MainPackage < PackageSpec
  strict_exports true
  export MainPackage::ClassWithMethod
end
