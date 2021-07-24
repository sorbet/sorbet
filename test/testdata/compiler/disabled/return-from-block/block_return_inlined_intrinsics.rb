# frozen_string_literal: true
# typed: true
# compiled: true

def f
  puts (42.times { return 42 })
  raise "Expected to return from f"
end

puts (f + 1000)

class C
  def self.f
    puts (42.times { return 45 })
    raise "expected to return from C.f"
  end

  def g
    puts (42.times { return 46 })
    raise "expected to return C#g"
  end
end

puts (C.f + 2000)
puts (C.new.g + 3000)

def k
  puts ([26,27,28,29].each { |x| return (x*2) })
  raise "Expected to return from k"
end

puts (k + 8000)

# No output is expected from the following, and we should return from static init.
puts (42.times { return 52 })
raise "Expected to return from file root"
