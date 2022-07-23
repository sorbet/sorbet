# typed: true

class A
  extend T::Sig

  sig {
    type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .returns(T.type_parameter(:U))
  }
  def generic_nested(x)
    x
  end

  sig {
    type_parameters(:U)
    .params(x: T.all(T.type_parameter(:U), T.any(String, Integer)))
    .returns(T.all(T.type_parameter(:U), T.any(String, Integer)))
  }
  def generic(x)
    generic_nested(x)
  end

  def test
    v1 = generic(1017)
    T.reveal_type(v1) # error: Revealed type: `Integer`
    T.let(v1, Integer)

    v2 = generic("hi")
    T.reveal_type(v2) # error: Revealed type: `String`
    T.let(v2, String)
  end
end
