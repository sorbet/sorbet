# typed: strict

class Foo::Bar::FalsePackage < PackageSpec
  layer 'lib'
  strict_dependencies 'false'
  export Foo::Bar::FalsePackage::OtherClass
  export Foo::Bar::FalsePackage::ImportMeTestOnly
  export Test::Foo::Bar::FalsePackage::TestUtil
end
