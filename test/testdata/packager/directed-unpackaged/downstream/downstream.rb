# typed: strict

# stratum: 0

module Downstream
  # this is not imported, but that means that this stratum doesn't
  # know anything about it, so it manifests as an unresolved constant
  MyPackage::MyClass
# ^^^^^^^^^ error: Unable to resolve

  # because this was defined in `MyPackage`, we can't have seen it yet
  UnpackagedTheSequel
# ^^^^^^^^^^^^^^^^^^^ error: Unable to resolve

  # this is defined by an RBI and not imported
  MyPackage::MyRbiConstant
# ^^^^^^^^^ error: Unable to resolve

  # because this was defined in `MyPackage`, we can't have seen it yet
  SomethingCompletelyDifferent
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve
end
