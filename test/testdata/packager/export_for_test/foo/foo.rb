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
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Used test-only constant `Test::Opus::Foo::Bar::BarClassTest` in non-test file

  # via import Opus::Util
  Opus::Util::UtilClass
  Test::Opus::Util::TestUtil
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Used test-only constant `Test::Opus::Util::TestUtil` in non-test file

  Opus::Util::Nesting::Public.public_method

  # util/__package.rb exposed via export_for_test, cannot access from here:
  Opus::Util::Nesting.nesting_method # error: Package `Opus::Util` does not export `Opus::Util::Nesting`


  # via test_import Opus::TestImported
  Opus::TestImported::TIClass
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Used `test_import` constant `Opus::TestImported::TIClass` in non-test file
  Test::Opus::TestImported::TITestClass
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Used `test_import` constant `Test::Opus::TestImported::TITestClass` in non-test file
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Used test-only constant `Test::Opus::TestImported::TITestClass` in non-test file


  # via export_for_test Opus::Foo::Private::ImplDetail
  Opus::Foo::Private::ImplDetail.stub_stuff!

  # Visible because it's local to this package
  Opus::Foo::FooUnexported
end
