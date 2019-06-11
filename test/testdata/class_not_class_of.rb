# typed: true

extend T::Sig

sig {params(c: Class).void}
def take_class(c)
  T.reveal_type(c) # error: Revealed type: `Class`
  if c === c
    puts
  else
    puts # error: This code is unreachable
  end
end
