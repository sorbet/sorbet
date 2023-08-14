# typed: strict

class Foo::Bar::OtherPackage < PackageSpec
  export Foo::Bar::OtherPackage::OtherClass
  export Foo::Bar::OtherPackage::ImportMeTestOnly
  export Test::Foo::Bar::OtherPackage::TestUtil
end
