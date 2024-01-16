# typed: strict

class Test::Opus::Foo::FooTest
  # Check Visibility
  # via import Opus::Foo::Bar
  Opus::Foo::Bar::BarClass
  Test::Opus::Foo::Bar::BarClassTest

  # via import Opus::Util
  Opus::Util::UtilClass
  Test::Opus::Util::TestUtil
  Opus::Util::Nesting::Public.public_method

  # util/__package.rb exposed via export_for_test, cannot access from here:
  Opus::Util::Nesting.nesting_method
# ^^^^^^^^^^^^^^^^^^^ error: `Opus::Util::Nesting` resolves but is not exported from `Opus::Util`

  # via test_import Opus::TestImported
  Opus::TestImported::TIClass
  Test::Opus::TestImported::TITestClass

  # Test code gets all public exports from its corresponding package
  # via export Opus::Foo::FooClass
  Opus::Foo::FooClass
  Opus::Foo::FooClass::Inner

  # via export_for_test Opus::Foo::Private::ImplDetail
  Opus::Foo::Private::ImplDetail.stub_stuff!

  # Visible, as the test package is able to see all of the package it's testing
  Opus::Foo::FooUnexported
end
