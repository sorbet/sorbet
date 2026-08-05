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


  # Check Visibility
  # via import Opus::Foo::Bar
  Opus::Foo::Bar::BarClass
  Test::Opus::Foo::Bar::BarClassTest
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Opus::Foo::Bar::BarClassTest` cannot be referenced here because `Opus::Foo` may not reference `test!` packages

  # via import Opus::Util
  Opus::Util::UtilClass
  Test::Opus::Util::TestUtil
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Opus::Util::TestUtil` cannot be referenced here because `Opus::Foo` may not reference `test!` packages

  Opus::Util::Nesting::Public.public_method

  # Not exported from util
  Opus::Util::Nesting.nesting_method
# ^^^^^^^^^^^^^^^^^^^ error: `Opus::Util::Nesting` resolves but is not exported from `Opus::Util`


  # TestImported is not imported by this package
  Opus::TestImported::TIClass
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Opus::TestImported::TIClass` resolves but its package is not imported
  Test::Opus::TestImported::TITestClass
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Opus::TestImported::TITestClass` cannot be referenced here because `Opus::Foo` may not reference `test!` packages


  # Private::ImplDetail is local to this package
  Opus::Foo::Private::ImplDetail.stub_stuff!

  # Visible because it's local to this package
  Opus::Foo::FooUnexported
end
