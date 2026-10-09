# typed: strict

# stratum: 0

class Parent < PackageSpec
  export Parent::MyClass

  test_import Child
end
