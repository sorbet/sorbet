# typed: true

class Foo
  def bar(a, b, k1:, k2:, &block); end

  def foo(...)
    T.unsafe(self).bar(...)
  end

  def foo2(*args, **kwargs, &block)
    T.unsafe(self).bar(*args, **kwargs, &block)
  end

  def foo3(...)
    T.unsafe(self).bar(*[1, 2, 3], ...)
  end

  def foo4(*args, **kwargs, &block)
    T.unsafe(self).bar(*[1, 2, 3], *args, **kwargs, &block)
  end
end

Foo.new.foo
Foo.new.foo(1, 2)
Foo.new.foo(k1: 1, k2: 2)
Foo.new.foo do puts "foo" end
Foo.new.foo(1, 2, k1: 1, k2: 2) do puts "foo" end
