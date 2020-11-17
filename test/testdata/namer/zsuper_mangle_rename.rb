# typed: true

# Either both these cases should be mangle rename errors, or neither.

class NoZSuperPrivatePublic
  def foo; end

  private :foo
  public :foo
end

class Parent
  def foo; end
end
class Child < Parent
  private :foo
  public :foo
end
