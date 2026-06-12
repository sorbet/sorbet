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

  # Not exported from util, and util is not imported with uses_internals
  Opus::Util::Nesting.nesting_method
# ^^^^^^^^^^^^^^^^^^^ error: `Opus::Util::Nesting` resolves but is not exported from `Opus::Util`

  # via import Opus::TestImported
  Opus::TestImported::TIClass
  Test::Opus::TestImported::TITestClass

  # Test code gets all public exports from its corresponding package
  # via export Opus::Foo::FooClass
  Opus::Foo::FooClass
  Opus::Foo::FooClass::Inner

  # via uses_internals: true
  Opus::Foo::Private::ImplDetail.stub_stuff!

  # Visible, as the test package uses_internals
  Opus::Foo::FooUnexported
end
