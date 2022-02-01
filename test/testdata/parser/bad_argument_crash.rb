# typed: true
extend T::Sig

sig {type_parameters(:).void}
#                    ^ error: unexpected token ":"
#                    ^ error: Unexpected `ConstantLit`
def ex1; end

sig {type_parameters(x).void}
#                    ^ error: Unexpected `Send`
#                    ^ error: Method `x` does not exist
def ex2; end

sig {type_parameters(X).void}
#                    ^ error: Unable to resolve constant `X`
#                    ^ error: Unexpected `ConstantLit`
def ex3; end

y = nil
sig {type_parameters(y).void}
#                    ^ error: Unexpected `Local`
def ex4; end
