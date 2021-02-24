# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

def destructuring_from(description, &blk)
  obj = yield

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

destructuring_from('nil') do
  nil
end

destructuring_from('integer') do
  17
end

destructuring_from('empty array') do
  [].freeze
end

destructuring_from('small array') do
  [9, 7].freeze
end

destructuring_from('large array') do
  [:ok, "6", 15, 3.8, "twenty", 45, :sixty].freeze
end

class ImplementsToAry
  def to_ary
    [:why, :ruby, :why].freeze
  end
end

destructuring_from('obj implementing to_ary') do
  ImplementsToAry.new
end
