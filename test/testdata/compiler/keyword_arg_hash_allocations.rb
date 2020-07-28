# frozen_string_literal: true
# typed: true
# compiled: true

class Main
  def self.main(arg0: 42, arg1: 253)
    [arg0, arg1]
  end
end

before = GC.stat.fetch(:total_allocated_objects)

# Since these keyword arguments appear as a Hash literal in the call,
# the Ruby VM optimizes this use case to avoid allocating a hash, instead
# using the individual values to populate the callee's locals directly.
Main.main(arg0: 24, arg1: 352)

n = GC.stat.fetch(:total_allocated_objects) - before - 1 # Subtract one for the allocation by GC.stat itself

puts "allocations=#{n}"
