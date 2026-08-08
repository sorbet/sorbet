# typed: true
# frozen_string_literal: true
require_relative '../../../lib/sorbet-runtime'

# After run_all_sig_blocks, the first call to an aliased method should have
# the same allocation cost as the first call to a non-aliased method.
# This ensures that aliases are fully unwrapped eagerly, rather than
# deferring unwrapping to first call (which incurs extra allocations).

class Example
  extend T::Sig

  sig { params(x: Symbol).returns(Symbol).checked(:never) }
  def foo(x=:foo)
    x
  end
  alias_method :bar, :foo

  sig { params(x: Symbol).returns(Symbol).checked(:never) }
  def baz(x=:foo)
    x
  end
end

T::Utils.run_all_sig_blocks

gc = GC

# Use fresh instances so both methods hit first-call paths together,
# controlling for Ruby VM inline-cache warmup allocations.
obj1 = Example.new
obj1.foo
before = gc.stat(:total_allocated_objects)
obj1.foo
foo_allocs = gc.stat(:total_allocated_objects) - before

obj2 = Example.new
obj2.bar
before = gc.stat(:total_allocated_objects)
obj2.bar
bar_allocs = gc.stat(:total_allocated_objects) - before

obj3 = Example.new
obj3.baz
before = gc.stat(:total_allocated_objects)
obj3.baz
baz_allocs = gc.stat(:total_allocated_objects) - before

if foo_allocs == bar_allocs && baz_allocs == bar_allocs
  puts "PASS"
else
  puts "FAIL: allocations were foo=#{foo_allocs}, bar=#{bar_allocs}, baz=#{baz_allocs} (all should be equal)"
  exit 1
end
