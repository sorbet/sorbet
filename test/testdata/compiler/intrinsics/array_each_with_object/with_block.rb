# frozen_string_literal: true
# typed: true
# compiled: true

a = T.let([1, 2, 3, 4, 5], T::Array[Integer])

state = [{}, 0]

result = a.each_with_object(state) do |x, state|
  state[0][x] = state[1]
  state[1] += 1
end

p result
p state

state = [{}, 0]

result = a.each_with_object(state) do |x, state|
  state[0][x] = state[1]
  state[1] += 1
  break if x == 2
end

p result
p state
