# typed: true
extend T::Sig

sig {params(x: T.nilable(String)).returns(String)}
def return_arg(x)
  x # error: Expected `String` but found `T.nilable(String)` for method result type
end

module M
  X = T.let(nil, T.nilable(String))
end

sig {returns(String)}
def return_constant_path
  M::X
# ^ error: Expected `String` but found `T.nilable(String)` for method result type
end
