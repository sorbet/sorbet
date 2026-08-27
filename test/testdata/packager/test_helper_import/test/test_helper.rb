# typed: strict

module Test::RootPkg
  class ExampleHelper
    A::Thing # allowed

    B::Thing # allowed---test helper import
    B::PrivateThing
  # ^^^^^^^^^^^^^^^ error: `B::PrivateThing` resolves but is not exported from `B`

    C::Thing # not allowed---test-only import
  # ^^^^^^^^ error: The `test_import` constant `C::Thing` can only be used in `.test.rb` files
    C::PrivateThing
  # ^^^^^^^^^^^^^^^ error: `C::PrivateThing` resolves but is not exported from `C` and `C` is `test_import`ed for only .test.rb files
  end
end
