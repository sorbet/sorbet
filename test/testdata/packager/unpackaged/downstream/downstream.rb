# typed: strict

module Downstream
  # this is not imported:
  MyPackage::MyClass
# ^^^^^^^^^^^^^^^^^^ error: is not imported

  # this was defined in the `MyPackage` code but defines things on
  # another namespace, so okay:
  UnpackagedTheSequel

  # this is defined by an RBI but not imported
  MyPackage::MyRbiConstant
# ^^^^^^^^^^^^^^^^^^^^^^^^ error: is not imported

  # this is defined by a packaged RBI but on another namespace
  SomethingCompletelyDifferent
end
