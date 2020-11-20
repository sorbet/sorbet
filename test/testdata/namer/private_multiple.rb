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

A.new.foo # Non-private call to private method `A#foo`
A.new.bar # Non-private call to private method `A#bar`
A.new.qux # Non-private call to private method `A#qux`

p A::X # Non-private reference to private constant `X`
p A::Y # Non-private reference to private constant `Y`
p A::Z # Non-private reference to private constant `Z`
