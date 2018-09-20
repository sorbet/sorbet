# typed: true
A = Class.new

# Ensure that `B` is marked as resolved; Previously we wouldn't
# resolve it because we replace `A` with `StubClass` and we would
# interpret B as unresolved.

class B < A # error: Superclasses and mixins must be statically typeAlias to classes
end
