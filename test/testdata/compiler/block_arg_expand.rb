# frozen_string_literal: true
# typed: true
# compiled: true

a = [[1, 2]]

# if a block/proc is given a single array as an arg and it expects more than a single arg, array is expanded to arguments
a.each do |el1, el2|
  T.let(el1, Integer) + T.let(el2, Integer)
end

# but if the block has a single arg, the array is not expanded
a.each do |array|
  p array
end

# also not expanded for a single optional defaulted arg
a.each do |array=:x|
  p array
end

# but expanded in the case of positional/optional combo
a.each do |x=:default, y=:something|
  p x
  p y
end

a.each do |x, y=:something|
  p x
  p y
end
