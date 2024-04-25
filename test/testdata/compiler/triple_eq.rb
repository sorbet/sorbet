# frozen_string_literal: true
# typed: true
# compiled: true

puts '-- Integer --'
puts Integer.===(1)
puts Integer.===('')

puts
puts '-- Class --'
x = T.let(Integer, Class)
puts x.===(1)
puts x.===('')

puts
puts '-- Parent --'
class Parent; end
class Child1 < Parent; end
class Child2 < Parent; end

y = T.let(Child1, T.class_of(Parent))
puts y.===(Child2.new)
