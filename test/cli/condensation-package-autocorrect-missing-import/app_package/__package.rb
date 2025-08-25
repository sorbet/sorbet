# typed: strict

class Foo::Bar::AppPackage < PackageSpec
  layer 'app'
  strict_dependencies 'layered'
  export Foo::Bar::AppPackage::OtherClass
  export Foo::Bar::AppPackage::ImportMeTestOnly
  export Test::Foo::Bar::AppPackage::TestUtil
end
