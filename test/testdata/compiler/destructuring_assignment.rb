# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

def destructuring_from(description, obj)
  puts "assignments from #{description}\n"
  puts "regular variables"
  first, _ = obj
  p first, _

  puts "array in the middle"
  first, *second, third = obj
  p first, second, third

  puts "array at the beginning"
  *most, last = obj
  p most, last

  puts "array at the end"
  first, *rest = obj
  p first, rest

  puts "array favoring left"
  a, *b, c, d, e, f = obj
  p a, b, c, d, e, f

  puts "array favoring right"
  a, b, c, *d, e, f = obj
  p a, b, c, d, e, f
end

destructuring_from('nil', nil)

destructuring_from('integer', 17)

destructuring_from('empty array', [].freeze)

destructuring_from('small array', [9, 7].freeze)

destructuring_from('large array', [:ok, "6", 15, 3.8, "twenty", 45, :sixty].freeze)

class ImplementsToAry
  def to_ary
    [:why, :ruby, :why].freeze
  end
end

destructuring_from('obj implementing to_ary', ImplementsToAry.new)
