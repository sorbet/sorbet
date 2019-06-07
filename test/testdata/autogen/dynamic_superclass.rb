# typed: true

# This is only an error because autogen does **not** run the ClassNew DSL pass.

A = Class.new

# Ensure that `B` is marked as resolved; Previously we wouldn't
# resolve it because we replace `A` with `StubModule` and we would
# interpret B as unresolved.

class B < A # error: Superclasses and mixins may only use class aliases like `A = Integer`
end
