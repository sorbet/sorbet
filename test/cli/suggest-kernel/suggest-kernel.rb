# typed: true
module Foo
  def foo
    raise "hi"
    Integer(0)
    Array(0)
  end
end

extend T::Sig

sig { params(x: Foo).void }
def example(x)
  x.is_a?(Foo)
end
