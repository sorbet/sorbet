# compiled: true
# typed: true
# frozen_string_literal: true

def f(c: 3)
  p (c)
end

begin
  f(*[{c: 10, d: 8}])
rescue ArgumentError => e
  p e.message
end
