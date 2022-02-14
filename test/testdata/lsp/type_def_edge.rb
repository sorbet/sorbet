# typed: true

extend T::Sig

sig {type_parameters(:U).params(x: T.type_parameter(:U)).void}
#                    ^^ type-def: test_self_type_param
def test_self_type_param(x)
  x
# ^ type: test_self_type_param
end

class A
end

class Generic
  extend T::Sig
  extend T::Generic
  X = type_member
# ^^^^^^^^^^^^^^^ type-def: test_lambda_param

  sig {params(x: X).void}
  #              ^ type: test_lambda_param
  def test_lambda_param(x)
    x
  end
end

class Wrapper
  class Normal
    extend T::Sig
    extend T::Generic

    sig {void}
    def test_nothings
      puts(:foo)
      #      ^ type: (nothing)

      shape = {foo: 0}
      puts(shape)
      #    ^ type: (nothing)

      tuple = [0]
      puts(tuple)
      #    ^ type: (nothing)

      meta = T.nilable(Integer)
      puts(meta)
      #    ^ type: (nothing)
    end

  end
end
