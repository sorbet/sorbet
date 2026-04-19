# typed: strong
extend T::Sig

sig {params(pair: T.any([Symbol], [Symbol, Symbol])).void}
def foo(pair)
  T.reveal_type(pair) # error: `T.any([Symbol], [Symbol, Symbol])`
  x, y = pair
  T.reveal_type(x) # error: `Symbol`
  T.reveal_type(y) # error: `T.nilable(Symbol)`
end
