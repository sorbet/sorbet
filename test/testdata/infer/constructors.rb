# typed: true
class Foo1
end

class Foo2
  def self.new(a)
  end
end

class Foo3
  def initialize(a, b)
  end
end

class Bar
  def foo
    Foo1.new
    Foo1.new(1)
    #        ^ error: Wrong number of arguments for constructor
    Foo2.new(1)
    Foo3.new(1, 2)

    Foo1.new(1) {}
    #        ^ error: Wrong number of arguments for constructor
    #          ^^^ error: Method `BasicObject#initialize` does not take a block
    Foo1.new 1 do end
    #        ^ error: Wrong number of arguments for constructor
    #         ^^^^^^^ error: Method `BasicObject#initialize` does not take a block
  end
end

class InstanceMethod
  def test_imethod_new
    o = Object.new
    o.new # error: Method `new` does not exist
  end

  def _; end

  class A; end
  class B; end

  def test_ortype_new
    a = if _ then A else B end
    T.assert_type!(a.new, T.any(A, B))
  end
end
