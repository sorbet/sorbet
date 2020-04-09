# typed: true

# The correctness of this test relies on our RBI definitions for NilClass#nil?,
# TrueClass#!, and FalseClass#!

a = nil

if !a.nil?
  puts '' # error: This code is unreachable
end

unless a.nil?
  puts '' # error: This code is unreachable
end

if !!a.nil?
  puts 'reachable'
else
  puts '' # error: This code is unreachable
end

unless !a.nil?
  puts 'reachable'
else
  puts '' # error: This code is unreachable
end

def pointless_type_check
  foo = 1
  if !foo.is_a?(Numeric)
    puts 'Must be Numeric' # error: This code is unreachable
  end
end
