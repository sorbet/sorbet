# typed: strict

module Test::Foo::MyPackage
  Test::Foo::Bar::CyclePackage::TestUtil

  Foo::Bar::CyclePackage::ImportMeTestOnly
end
