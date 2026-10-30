# typed: strict

module RootPkg
  class Example
    A::Thing # allowed

    B::Thing # not allowed---test import
  # ^ error: Used `test_import` package `B` in non-test file
    B::PrivateThing
  # ^ error: Used `test_import` package `B` in non-test file

    C::Thing # not allowed---test import
  # ^ error: Used `test_import` package `C` in non-test file
    C::PrivateThing
  # ^ error: Used `test_import` package `C` in non-test file
  end
end
