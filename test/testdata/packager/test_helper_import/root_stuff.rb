# typed: strict

module RootPkg
  class Example
    A::Thing # allowed

    B::Thing # not allowed---test import
  # ^^^^^^^^ error: Used `test_import` constant `B::Thing` in non-test file
    B::PrivateThing
  # ^^^^^^^^^^^^^^^ error: `B::PrivateThing` resolves but is not exported from `B` and `B` is `test_import`ed

    C::Thing # not allowed---test import
  # ^^^^^^^^ error: Used `test_import` constant `C::Thing` in non-test file
    C::PrivateThing
  # ^^^^^^^^^^^^^^^ error: `C::PrivateThing` resolves but is not exported from `C` and `C` is `test_import`ed
  end
end
