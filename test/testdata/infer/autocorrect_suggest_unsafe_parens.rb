# typed: true
# enable-suggest-unsafe: true
extend T::Sig

sig { params(x: Integer).void }
def takes_int(x); end

sig { params(u: T.untyped).void }
def example(u)
  takes_int((u; ''))
  #          ^^^^^ error: Expected `Integer` but found `String("")` for argument `x`

  takes_int((''))
  #          ^^ error: Expected `Integer` but found `String("")` for argument `x`

  takes_int(((u; '')))
  #           ^^^^^ error: Expected `Integer` but found `String("")` for argument `x`

  takes_int(begin (u; '') end)
  #                ^^^^^ error: Expected `Integer` but found `String("")` for argument `x`

  (u; nil).foo
  #        ^^^ error: Method `foo` does not exist on `NilClass`
end

class ParensInClassBody
  (1; 2)
end
