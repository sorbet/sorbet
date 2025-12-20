# frozen_string_literal: true
# typed: true
# compiled: true

nil_ = nil
str = ''

i = 0

nil_isa_nil = T.let(nil, T.nilable(T::Boolean))
str_isa_str = T.let(nil, T.nilable(T::Boolean))

str_isa_nil = T.let(nil, T.nilable(T::Boolean))
nil_isa_str = T.let(nil, T.nilable(T::Boolean))

nil_isa_obj = T.let(nil, T.nilable(T::Boolean))
str_isa_obj = T.let(nil, T.nilable(T::Boolean))

while i < 10_000_000

  # Classes are equal, so ancestor search will exit early (in interpreter)

  nil_isa_nil = nil_.is_a?(NilClass)
  str_isa_str = str.is_a?(String)

  # When the result is false the VM never exits early (inspects every ancestor)

  str_isa_nil = str.is_a?(NilClass)
  nil_isa_str = nil_.is_a?(String)

  # Classes are not equal, but result is true (partial ancestor search)

  nil_isa_obj = nil_.is_a?(Object)
  str_isa_obj = str.is_a?(Object)

  i += 1
end

puts "iterations: #{i}"
puts
puts "nil_.is_a?(NilClass) => #{nil_isa_nil}"
puts "str.is_a?(String)    => #{str_isa_str}"
puts
puts "str.is_a?(NilClass)  => #{str_isa_nil}"
puts "nil_.is_a?(String)   => #{nil_isa_str}"
puts
puts "nil_.is_a?(Object)   => #{nil_isa_obj}"
puts "str.is_a?(Object)    => #{str_isa_obj}"
