
# frozen_string_literal: true
# typed: true
# compiled: true

puts '-- Integer --'
puts case T.let(1, T.any(Integer, String))
when Integer
  'int'
else
  'not int'
end

puts case T.let('', T.any(Integer, String))
when Integer
  'int'
else
  'not int'
end

puts
puts '-- Class --'
x = T.let(Integer, Class)
case 1
when x
  puts 'is x'
else
  puts 'is not x'
end
case ''
when x
  puts 'is x'
else
  puts 'is not x'
end

puts
puts '-- Parent --'
class Parent; end
class Child1 < Parent; end
class Child2 < Parent; end

y = T.let(Child1, T.class_of(Parent))
puts case Child2.new
when y
  'is y'
end
