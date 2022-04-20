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
# ^^^^ error: Unable to resolve constant `Test`


  # via import Opus::Util
  Opus::Util::UtilClass
  Test::Opus::Util::TestUtil
# ^^^^ error: Unable to resolve constant `Test`
  #
  Opus::Util::Nesting::Public.public_method

  # util/__package.rb exposed via export_for_test, cannot access from here:
  Opus::Util::Nesting.nesting_method
  #                   ^^^^^^^^^^^^^^ error: Method `nesting_method` does not exist on `T.class_of(Opus::Util::Nesting)`


  # via test_import Opus::TestImported
  Opus::TestImported::TIClass
# ^^^^^^^^^^^^^^^^^^ error: No import provides `Opus::TestImported`
  Test::Opus::TestImported::TITestClass
# ^^^^ error: Unable to resolve constant `Test`


  # via export_for_test Opus::Foo::Private::ImplDetail
  Opus::Foo::Private::ImplDetail.stub_stuff!

  # Visible because it's local to this package
  Opus::Foo::FooUnexported
end
