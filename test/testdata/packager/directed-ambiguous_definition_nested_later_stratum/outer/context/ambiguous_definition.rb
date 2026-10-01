# typed: true

module Outer
  module Context
    # `Outer::A` is in a later stratum, so package-directed mode does not report
    # the ambiguity from this non-root lexical scope.
    module A::B
    end
  end
end
