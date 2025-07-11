# typed: strict

module Test::RootPkg
  class ExampleHelper
    A::Thing # allowed

    B::Thing # allowed---test helper import

    C::Thing # not allowed---test-only import
  # ^^^^^^^^ error: The `test_import` constant `C::Thing` can only be used in `.test.rb` files
  end
end
