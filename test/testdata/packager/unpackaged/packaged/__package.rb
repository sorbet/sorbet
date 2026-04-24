# frozen_string_literal: true
# typed: strict

class MyPackage < PackageSpec
  import OtherPackageImported
  export MyPackage::MyClass
end
