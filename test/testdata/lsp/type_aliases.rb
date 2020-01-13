# typed: true
class BigFoo
    # ^ def: BigFoo
  extend T::Sig
end

BigFooAlias = T.type_alias {BigFoo}
# ^ hover: <Type: BigFoo>
# ^ def: BigFooAlias
                          # ^ usage: BigFoo

def main
  l = T.let(nil, T.nilable(BigFooAlias))
# ^ hover: T.nilable(BigFoo)
                         # ^ hover: <Type: BigFoo>
                         # ^ usage: BigFooAlias
end
