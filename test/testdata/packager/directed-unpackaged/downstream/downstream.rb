# typed: strict

# stratum: 0

module Downstream
  # The package registry lets the resolver identify this package even though its source stratum has not run.
  MyPackage::MyClass
# ^^^^^^^^^ error: `MyPackage` is not imported

  # because this was defined in `MyPackage`, we can't have seen it yet
  UnpackagedTheSequel
# ^^^^^^^^^^^^^^^^^^^ error: Unable to resolve

  # this is defined by an RBI and not imported
  MyPackage::MyRbiConstant
# ^^^^^^^^^ error: `MyPackage` is not imported

  # because this was defined in `MyPackage`, we can't have seen it yet
  SomethingCompletelyDifferent
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve
end
