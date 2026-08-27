# frozen_string_literal: true
# typed: strict

class MyPackage::MyRbiConstant
end

# This will be an error because it defines a constant which does
# not match the enclosing namespace
class SomethingElse
    # ^^^^^^^^^^^^^ error: defines a constant that does not match this namespace
end

# This will be okay because of the leading `::`
class ::SomethingCompletelyDifferent
end
