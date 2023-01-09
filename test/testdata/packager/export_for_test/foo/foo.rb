# typed: strict

module Opus::Foo
  class FooClass # exported publicly
    Inner = 1
  end

  class FooUnexported; end

  class Private::ImplDetail
    extend T::Sig
    sig {void}
    def self.stub_stuff!; end
  end


  # Check Visiblity
  # via import Opus::Foo::Bar
  Opus::Foo::Bar::BarClass
  Test::Opus::Foo::Bar::BarClassTest
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Opus::Foo::Bar::BarClassTest` is defined in a test namespace

  # via import Opus::Util
  Opus::Util::UtilClass
  Test::Opus::Util::TestUtil
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Opus::Util::TestUtil` is defined in a test namespace

  Opus::Util::Nesting::Public.public_method

  # util/__package.rb exposed via export_for_test, cannot access from here:
  Opus::Util::Nesting.nesting_method
# ^^^^^^^^^^^^^^^^^^^ error: `Opus::Util::Nesting` resolves but is not exported from `Opus::Util`


  # via test_import Opus::TestImported
  Opus::TestImported::TIClass
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Used `test_import` constant `Opus::TestImported::TIClass` in non-test file
  Test::Opus::TestImported::TITestClass
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Used `test_import` constant `Test::Opus::TestImported::TITestClass` in non-test file
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Opus::TestImported::TITestClass` is defined in a test namespace


  # via export_for_test Opus::Foo::Private::ImplDetail
  Opus::Foo::Private::ImplDetail.stub_stuff!

  # Visible because it's local to this package
  Opus::Foo::FooUnexported
end
