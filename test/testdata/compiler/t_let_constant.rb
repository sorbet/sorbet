# frozen_string_literal: true
# typed: true
# compiled: true

def f
  hash = T.let({a: 1, b: 1, c: 2, d: 3, e: 5, f: 8, g: 13}, T::Hash[Symbol, Integer])
end

p f

def g
  array = T.let([1, 2, 3, 4], T::Array[Integer])
end

p g
