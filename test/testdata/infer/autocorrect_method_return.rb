# typed: true
extend T::Sig

sig {params(x: T.nilable(String)).returns(String)}
def return_arg(x)
  x # error: Expected `String` but found `T.nilable(String)` for method result type
end
