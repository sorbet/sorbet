# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(queue: T::Array[T.untyped], obj: T::Array[T.untyped]).void}
def f(queue, obj)
  obj.each do |x|
    queue << x
  end
end

q = []
f(q, [1, 2, 3])
f(q, [:z, :y, :x, :w])
p q

# Check that:
#
# * We read the local
# * We applied an appropriate llvm.assume to it
# * We did a normal type test + branch to intrinsic/VM implementations
#
# The last is not strictly necessary, but it helps ensure that the VM-based codepath
# is there so that if it disappeared in some way, we have a warning besides just the
# OPT-NOT test below succeeding.

# We should have optimized out the alternative VM-based call path based on type assumptions.
