# typed: true

class Test
  extend T::Sig

  sig do
    type_parameters(:T)
      .params(x: T.type_parameter(:T))
      .returns(T.type_parameter(:T))
  end
  def test(x)
    if x.is_a? Integer
      T.reveal_type(x) # error: Revealed type: `T.all(Integer, Test#test#T)`
    end
    x
  end
end
