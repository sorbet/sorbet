# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

# When `extend T::Sig` in a random class happens before the `include T::Sig` in
# `Module`, there will be multiple method_added hooks in the hierarchy.
#
# Since `sorbet-runtime` itself has some classes that do `extend T::Sig` (e.g.,
# in `T::Props` code), any codebases monkey patch of `Module` will always come
# after the first `extend T::Sig`.

# In practice, this comes from `module T::Props::ClassMethods`
class Parent
  extend T::Sig
end

class Module
  include T::Sig
end

# In practice, this comes from something like `include T::Props`
class Child < Parent
  sig { returns(Integer).checked(:never) }
  def foo; 1; end
end

gc = GC

obj = Child.new
obj.foo

before = gc.stat(:total_allocated_objects)
obj.foo
allocs1 = gc.stat(:total_allocated_objects) - before

before = gc.stat(:total_allocated_objects)
obj.foo
allocs2 = gc.stat(:total_allocated_objects) - before

if allocs1 == 1 && allocs2 == 0
  puts "PASS"
else
  puts "FAIL: allocations were [#{allocs1}, #{allocs2}] (expected [1, 0])"
  exit 1
end
