# frozen_string_literal: true
# typed: true
# compiled: true

class GrandParent
end

class Parent < GrandParent
end

class Child < Parent
end

grand_parent = GrandParent.new
parent = Parent.new
child = Child.new

nil_as_object = T.let(nil, Object)
int = T.let(0, Integer)
str = T.let('', String)
sym = T.let(:'', Symbol)

i = 0
while i < 10_000_000

  grand_parent.nil?
  parent.nil?
  child.nil?

  nil_as_object.nil?
  int.nil?
  str.nil?

  sym.nil?

  i += 1
end

puts i

p grand_parent.nil?
p parent.nil?
p child.nil?

p nil_as_object.nil?
p int.nil?
p str.nil?

p sym.nil?
