# typed: true

module Outer
  module Context
    # Exercises registry lookup from a non-root lexical scope.
    module A::B # error: Definition of `B` is possibly ambiguous
    end
  end
end
