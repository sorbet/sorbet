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
      #  ^^^^^ error: Call to method `is_a?` on unconstrained generic type `T.type_parameter(:T) (of Test#test)`
      T.reveal_type(x) # error: Revealed type: `T.all(Integer, T.type_parameter(:T) (of Test#test))`
      take_integer(x)
    else
      take_integer(x) # error: Expected `Integer` but found `T.type_parameter(:T) (of Test#test)`
    end
    x
  end
end
