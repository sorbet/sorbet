# typed: true

class Test
  extend T::Sig

  sig {params(x:Integer).void}
  def take_integer(x)
  end

  sig do
    type_parameters(:T)
      .params(x: T.type_parameter(:T))
      .returns(T.type_parameter(:T))
  end
  def test(x)
    if x.is_a? Integer
      T.reveal_type(x) # error: Revealed type: `T.all(Integer, Test#test#T)`
      take_integer(x)
    else
      take_integer(x) # error: Expected `Integer` but found `Test#test#T`
    end
    x
  end
end
