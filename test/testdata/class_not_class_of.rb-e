# typed: true

extend T::Sig

sig {params(c: Class).void}
def take_class(c)
  T.reveal_type(c) # error: Revealed type: `Class`
  c === c
end
