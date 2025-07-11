# typed: strict

class Foo::Bar::FalseAndAppPackageTest < PackageSpec
  layer 'app'
  strict_dependencies 'false'
  export Foo::Bar::FalseAndAppPackageTest::OtherClass
end
