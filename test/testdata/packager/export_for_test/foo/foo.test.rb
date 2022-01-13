# typed: strict

class Test::Opus::Foo::FooTest
  # Check Visiblity
  # via import Opus::Foo::Bar
  Opus::Foo::Bar::BarClass
  Test::Opus::Foo::Bar::BarClassTest

  # via import Opus::Util
  Opus::Util::UtilClass
  Test::Opus::Util::TestUtil
  Opus::Util::Nesting::Public.public_method

  # util/__package.rb exposed via export_for_test, cannot access from here:
  Opus::Util::Nesting.nesting_method
  #                   ^^^^^^^^^^^^^^ error: Method `nesting_method` does not exist on `T.class_of(Opus::Util::Nesting)`

  # via test_import Opus::TestImported
  Opus::TestImported::TIClass
  Test::Opus::TestImported::TITestClass

  # Test code gets all public exports from its corresponding package
  # via export Opus::Foo::FooClass
  Opus::Foo::FooClass
  Opus::Foo::FooClass::Inner

  # via export_for_test Opus::Foo::Private::ImplDetail
  Opus::Foo::Private::ImplDetail.stub_stuff!

  # Not exported at all from Foo
  Opus::Foo::FooUnexported
# ^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `FooUnexported`
end
