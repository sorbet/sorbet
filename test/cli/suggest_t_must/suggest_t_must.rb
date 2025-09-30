# typed: true

foo = T.let(nil, T.nilable(String))
foo[0]

"hi" + foo

T::Array[T.nilable(Integer)].new.map(&:even?)

class A
  extend T::Sig

  sig {void}
  def initialize
    @result = T.let(0, Integer)
  end

  sig {params(xs: T::Array[Integer], i: Integer).void}
  def main(xs, i)
    @result = xs[i]
  end
end

x, y = [1, T.let('', T.nilable(String))]
y.split

class B
  extend T::Sig

  sig {
    type_parameters(:U)
      .params(x: T.all(NilClass, T.type_parameter(:U)))
      .void
  }
  def nil_type_parameter(x)
    x.foo
  end
end
