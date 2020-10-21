# typed: true

class Foo
  extend T::Sig

  sig { params(a: String).returns(String) }
  def foo(a)
    a * 10
  end
end

Foo.new.foo("foo")
