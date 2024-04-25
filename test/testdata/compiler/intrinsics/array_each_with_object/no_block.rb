# frozen_string_literal: true
# typed: true
# compiled: true

a = T.let([1, 2, 3, 4, 5], T::Array[Integer])

state = {}

enum = a.each_with_object(state)

p enum.class

result = enum.with_index do |(x, state), i|
  state[x] = i
end

p result
p state
