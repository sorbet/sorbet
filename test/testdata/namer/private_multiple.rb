# typed: true

class A
  def foo; end
  def bar; end
  def qux; end

  private :foo, :bar, :qux

  X = 1
  Y = 2
  Z = 3

  private_constant :X, :Y, :Z
end

A.new.foo # error: Non-private call to private method `A#foo`
A.new.bar # error: Non-private call to private method `A#bar`
A.new.qux # error: Non-private call to private method `A#qux`

p A::X # error: Non-private reference to private constant `A::X`
p A::Y # error: Non-private reference to private constant `A::Y`
p A::Z # error: Non-private reference to private constant `A::Z`
