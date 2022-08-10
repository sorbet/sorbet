# typed: true

# This was previously a problem because autogen skipped the rewriter passes

A = Class.new

# Ensure that `B` is marked as resolved; Previously we wouldn't
# resolve it because we replace `A` with `StubModule` and we would
# interpret B as unresolved.

class B < A
end
