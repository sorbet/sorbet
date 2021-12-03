# typed: strict

class Foo::Bar::OtherPackage < PackageSpec
  export Foo::Bar::OtherPackage::OtherClass
  export Test::Foo::Bar::OtherPackage::TestUtil
end
