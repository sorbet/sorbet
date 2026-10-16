# typed: strict

# stratum: 2

class Parent < PackageSpec
  export Parent::MyGeneric
  export Parent::ParentAlias

  test_import Child
end
