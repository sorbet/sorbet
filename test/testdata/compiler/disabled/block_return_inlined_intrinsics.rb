# frozen_string_literal: true
# typed: true
# compiled: true

def f
  puts (42.times { return 42 })
  raise "Expected to return from f"
end

puts (f + 1000)

module M
  begin
    puts (42.times { return 43 })
    raise "Expected LocalJumpError (can't return from module)"
  rescue LocalJumpError
    puts "Got the LocalJumpError we expected (can't return from module)"
  end
end

class C
  begin
    puts (42.times { return 44 })
    raise "Expected LocalJumpError (can't return from class)"
  rescue LocalJumpError
    puts "Got the LocalJumpError we expected (can't return from class)"
  end

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

def g
  begin
    raise "yikes"
  rescue
    puts (42.times { return 47 })
    raise "Expected to return from g but we're still in the rescue"
  ensure
    puts "Hit the ensure in g"
    666
  end
  raise "Expected to return from g but we're still in g"
end

puts (g + 4000)

def h
  begin
    raise "yikes"
  rescue
    puts "Hit the rescue in h"
  ensure
    puts (42.times { return 48 })
    raise "Expected to return from h but we're still in the ensure"
  end
  raise "Expected to return from h but we're still in h"
end

puts (h + 5000)

def i
  begin
    raise "yikes"
  rescue
    puts (42.times { return 49 })
    raise "Expected to return from i but we're still in the rescue"
  ensure
    puts "Hit the ensure in i"
    puts (42.times { return 50 })
  end
  raise "Expected to return from i but we're still in i"
end

puts (i + 6000)

def j
  begin
    puts (42.times { return 51 })
  rescue
    raise "Expected to return from j but we're still in the rescue"
  ensure
    puts "Hit the ensure in j"
    777
  end
  raise "Expected to return from j but we're still in j"
end

puts (j + 7000)

def k
  puts ([26,27,28,29].each { |x| return (x*2) })
  raise "Expected to return from k"
end

puts (k + 8000)

# No output is expected from the following, and we should return from static init.
puts (42.times { return 52 })
raise "Expected to return from file root"
