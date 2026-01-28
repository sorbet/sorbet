# typed: strict

class Foo::Bar::OtherPackage < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'
  export Foo::Bar::OtherPackage::OtherClass
  export Foo::Bar::OtherPackage::ImportMeTestOnly

  visible_to "tests"
end
