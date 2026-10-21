# typed: strict

class WithTestPackage::Test < PackageSpec
  test!

  import WithTestPackage, uses_internals: true
end
