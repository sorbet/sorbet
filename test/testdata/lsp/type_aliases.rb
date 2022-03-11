# typed: true
class BigFoo
    # ^ def: BigFoo
  extend T::Sig
end

BigFooAlias = T.type_alias {BigFoo}
# ^ hover: T.type_alias {BigFoo}
# ^ def: BigFooAlias
                          # ^ usage: BigFoo

def main
  l = T.let(nil, T.nilable(BigFooAlias))
# ^ hover: T.nilable(BigFoo)
  #                        ^ hover: T.type_alias {BigFoo}
  #                        ^ usage: BigFooAlias
end
