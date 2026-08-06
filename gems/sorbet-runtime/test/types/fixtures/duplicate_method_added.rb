# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

# When `extend T::Sig` in a random class happens before the `include T::Sig` in
# `Module`, there will be multiple method_added hooks in the hierarchy.
#
# Since `sorbet-runtime` itself has some classes that do `extend T::Sig` (e.g.,
# in `T::Props` code), any codebases monkey patch of `Module` will always come
# after the first `extend T::Sig`.

class Parent
  extend T::Sig
end

# Step 2: Module.include(T::Sig) — as pay-server's extn/module.rb does.
# This prepends MethodHooks to Module, creating a second MethodHooks entry
# in the method_added super chain for classes that already extended T::Sig.
class Module
  include T::Sig
end

# Step 3: Define a sig'd method on the class.
# method_added will fire twice through MethodHooks (once from the class's
# own singleton chain, once from Module's prepend), and the sig must survive.
class Parent
  sig { returns(Integer).checked(:never) }
  def foo; 1; end
end

gc = GC

obj = Parent.new
obj.foo

before = gc.stat(:total_allocated_objects)
obj.foo
allocs1 = gc.stat(:total_allocated_objects) - before

before = gc.stat(:total_allocated_objects)
obj.foo
allocs2 = gc.stat(:total_allocated_objects) - before

# After the first call evaluates and unwraps the sig, subsequent calls
# should have 0 allocations (the wrapper must not remain installed).
if allocs1 == 1 && allocs2 == 0
  puts "PASS"
else
  puts "FAIL: allocations were [#{allocs1}, #{allocs2}] (expected [1, 0])"
  exit 1
end
