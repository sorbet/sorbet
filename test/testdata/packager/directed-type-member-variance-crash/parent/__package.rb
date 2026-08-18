# typed: strict

# stratum: 0

class Parent < PackageSpec
  export Parent::MyGeneric
  export Parent::ParentAlias

  test_import Child
end
