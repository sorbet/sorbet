# frozen_string_literal: true
# typed: true
# compiled: true

nil_ = T.unsafe(nil)
str = T.unsafe('')

i = 0

nil_eq_nil = T.let(nil, T.nilable(T::Boolean))
str_eq_str = T.let(nil, T.nilable(T::Boolean))

nil_eq_str = T.let(nil, T.nilable(T::Boolean))
str_eq_nil = T.let(nil, T.nilable(T::Boolean))

obj_eq_nil = T.let(nil, T.nilable(T::Boolean))
obj_eq_str = T.let(nil, T.nilable(T::Boolean))

# We're using `case` in this example instead of calling `.===` directly because
# the VM will use different bytecode instructions to handle a `case`.
#
# It will eventually dispatch to a method's `===` method if required, but it's
# more common to see `case` written in normal code, so this makes the benchmark
# somewhat more realistic.

while i < 10_000_000

  # Classes are equal, so ancestor search will exit early (in interpreter)

  nil_eq_nil =
    case nil_
    when NilClass then true
    else false
    end

  str_eq_str =
    case str
    when String then true
    else false
    end

  # When the result is false the VM never exits early (inspects every ancestor)

  nil_eq_str =
    case str
    when NilClass then true
    else false
    end


  str_eq_nil =
    case nil_
    when String then true
    else false
    end

  # Classes are not equal, but result is true (partial ancestor search)

  obj_eq_nil =
    case nil_
    when Object then true
    else false
    end

  obj_eq_str =
    case str
    when Object then true
    else false
    end

  i += 1
end

puts "iterations: #{i}"
puts
puts "NilClass.===(nil_) => #{nil_eq_nil}"
puts "String.===(str)    => #{str_eq_str}"
puts
puts "NilClass.===(str)  => #{nil_eq_str}"
puts "String.===(nil_)   => #{str_eq_nil}"
puts
puts "Object.===(nil_)   => #{obj_eq_nil}"
puts "Object.===(str)    => #{obj_eq_str}"
