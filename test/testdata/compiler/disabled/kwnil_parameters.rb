# frozen_string_literal: true
# typed: true
# compiled: true

def foo(a, **nil)
end

puts method(:foo).parameters
