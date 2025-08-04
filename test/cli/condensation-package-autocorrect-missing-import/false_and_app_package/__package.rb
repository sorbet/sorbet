# typed: strict

class Foo::Bar::FalseAndAppPackage < PackageSpec
  layer 'app'
  strict_dependencies 'false'
  export Foo::Bar::FalseAndAppPackage::OtherClass
  export Foo::Bar::FalseAndAppPackage::ImportMeTestOnly
  export Test::Foo::Bar::FalseAndAppPackage::TestUtil
end
