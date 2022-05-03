# typed: strict

class Parent < PackageSpec
  import Parent::Child

  export_for_test Parent
end
