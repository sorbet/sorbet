# typed: true

class A
  def foo; end
  def bar; end

  private :foo, :bar

  X = 1
  Y = 2

  private_constant :X, :Y
end

A.new.foo # Non-private call to private method `foo`
A.new.bar

p A::X # Non-private reference to private constant `X`
p A::Y
