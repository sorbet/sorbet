# typed: true

extend T::Sig

sig {void}
def returns_void
  T.unsafe(nil)
end

sig {void}
def main
  result = returns_void
  if result
    #^^^^^^ error: Branching on `void` value
    T.reveal_type(result) # error: Revealed type: `Sorbet::Private::Static::Void`
  else
    T.reveal_type(result) # error: This code is unreachable
  end
end
