# typed: true

class A
  extend T::Sig
  sig { params(x: T.self_type).returns(T.self_type) }
  def example(x)
    T.reveal_type(self)            # error: Revealed type: `A`
    T.let(self, T.self_type)

    xs = T::Array[T.self_type].new
    T.reveal_type(xs)              # error: Revealed type: `T::Array[A]`

    xs << self
    xs << 0                        # error: Expected `A` but found `Integer(0)` for argument `arg0`

    x0 = xs.fetch(0)
    T.reveal_type(x0)              # error: Revealed type: `A`

    T.let(x0, T.self_type)
    self
  end
end
