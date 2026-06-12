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
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Package `Opus::Foo` may not reference `test!` packages
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Opus::Foo::Bar::BarClassTest` resolves but its package is not imported

  # via import Opus::Util
  Opus::Util::UtilClass
  Test::Opus::Util::TestUtil
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Package `Opus::Foo` may not reference `test!` packages
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Opus::Util::TestUtil` resolves but its package is not imported

  Opus::Util::Nesting::Public.public_method

  # Not exported from util
  Opus::Util::Nesting.nesting_method
# ^^^^^^^^^^^^^^^^^^^ error: `Opus::Util::Nesting` resolves but is not exported from `Opus::Util`


  # TestImported is not imported by this package
  Opus::TestImported::TIClass
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Opus::TestImported::TIClass` resolves but its package is not imported
  Test::Opus::TestImported::TITestClass
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Package `Opus::Foo` may not reference `test!` packages
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Opus::TestImported::TITestClass` resolves but its package is not imported


  # Private::ImplDetail is local to this package
  Opus::Foo::Private::ImplDetail.stub_stuff!

  # Visible because it's local to this package
  Opus::Foo::FooUnexported
end
