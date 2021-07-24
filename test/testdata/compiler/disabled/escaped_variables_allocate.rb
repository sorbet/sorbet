# frozen_string_literal: true
# typed: true
# compiled: true

class Main
  def self.arg0_escapes(arg0)
    2.times do
      T.unsafe(arg0) # force this to escape and not be dead code eliminated
    end
    nil
  end

  def self.counting_allocations(&blk)
    before = GC.stat.fetch(:total_allocated_objects)
    yield
    GC.stat.fetch(:total_allocated_objects) - before - 1 # Subtract one for the allocation by GC.stat itself
  end
end

n = Main.counting_allocations do
  Main.arg0_escapes(440)
end

puts "allocations=#{n}"
