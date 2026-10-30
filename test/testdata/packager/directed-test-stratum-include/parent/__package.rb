# typed: strict

# stratum: 2

class Parent < PackageSpec
  export Parent::MyClass

  test_import Child
end
