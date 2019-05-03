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
