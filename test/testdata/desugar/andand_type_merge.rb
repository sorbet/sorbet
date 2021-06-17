# typed: strict

extend T::Sig

Pair = T.type_alias do
  T.any(Integer, String)
end

sig {params(x: Pair).returns(String)}
def create(x)
  case x
  when String
    x
  else
    x.to_s
  end
end

sig {params(pair: T.nilable(Pair)).void}
def f(pair)
  pair &&= create(pair)
  T.reveal_type(pair) # error: Revealed type: `T.nilable(String)`
end
