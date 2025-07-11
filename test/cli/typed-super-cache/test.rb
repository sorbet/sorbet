# typed: true

class Parent
  def foo
  end
end

class Child < Parent
  def foo
    super
  end
end
