# typed: strict

module RootPkg
  class Example
    A::Thing # allowed

    B::Thing # not allowed---test import
  # ^^^^^^^^ error: Used `test_import` constant `B::Thing` in non-test file

    C::Thing # not allowed---test import
  # ^^^^^^^^ error: Used `test_import` constant `C::Thing` in non-test file
  end
end
