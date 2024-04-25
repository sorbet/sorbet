# frozen_string_literal: true
# typed: true
# compiled: true

# This defines "yield_from_2", which simply yields with no block arg, and "yield_with_arg_from_2" which yields with
# the argument passed to the block:
require_relative './block_return_over_exception_frames_compiled_compiled__2'

module M
  begin
    puts (yield_from_2 { return 43 })
    raise "Expected LocalJumpError (can't return from module)"
  rescue LocalJumpError
    puts "Got the LocalJumpError we expected (can't return from module)"
  end
end

class C
  begin
    puts (yield_from_2 { return 44 })
    raise "Expected LocalJumpError (can't return from class)"
  rescue LocalJumpError
    puts "Got the LocalJumpError we expected (can't return from class)"
  end
end

def g
  begin
    raise "yikes"
  rescue
    puts "Hit the rescue in g"
    puts (yield_from_2 { return 47 })
    raise "Expected to return from g but we're still in the rescue"
  ensure
    puts "Hit the ensure in g"
    666
  end
end

puts (4000000 + g + 4000)

def h
  begin
    raise "yikes"
  rescue
    puts "Hit the rescue in h"
  ensure
    puts "Hit the ensure in h"
    puts (yield_from_2 { return 48 })
    raise "Expected to return from h but we're still in the ensure"
  end
  raise "Expected to return from h but we're still in h"
end

puts (5000000 + h + 5000)

def i
  begin
    raise "yikes"
  rescue
    puts "Hit the rescue in i"
    puts (yield_from_2 { return 49 })
    raise "Expected to return from i but we're still in the rescue"
  ensure
    puts "Hit the ensure in i"
    puts (yield_from_2 { return 50 })
  end
end

puts (6000000 + i + 6000)

def j
  begin
    puts (yield_from_2 { return 51 })
  rescue
    raise "Expected to return from j but we're still in the rescue"
  ensure
    puts "Hit the ensure in j"
    777
  end
  raise "Expected to return from j but we're still in j"
end

puts (7000000 + j + 7000)

def l
  begin
    raise "yikes"
  rescue
    begin
      raise "yowza"
    rescue
      puts (yield_from_2 { return 999 })
    end
  end
end

puts (9000000 + l + 9000)
