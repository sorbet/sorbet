# typed: strict

class Foo::Bar::OtherPackageTest < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'
  export Foo::Bar::OtherPackageTest::OtherClass
end
