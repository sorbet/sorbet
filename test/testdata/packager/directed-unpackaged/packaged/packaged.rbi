# frozen_string_literal: true
# typed: strict

class MyPackage::MyRbiConstant
end

# This will be an error because it defines a constant which does
# not match the enclosing namespace
class SomethingElse
    # ^^^^^^^^^^^^^ error: defines a constant that does not match this namespace
end

# Root-scoped constants may only opt out of package prefix checks in prelude packages.
class ::SomethingCompletelyDifferent # error: requires this package to be marked `prelude!`
end
