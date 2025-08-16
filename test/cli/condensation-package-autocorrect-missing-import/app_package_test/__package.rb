# typed: strict

class Foo::Bar::AppPackageTest < PackageSpec
  layer 'app'
  strict_dependencies 'layered'
  export Foo::Bar::AppPackageTest::OtherClass
end
