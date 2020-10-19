# typed: true

extend T::Sig

module Boolean
  sealed! # error: `sealed!` does not exist
end

sig {params(x: Boolean).void}
def foo(x)
  case x
  when TrueClass
    T.reveal_type(x) # error: Revealed type: `T.all(Boolean, TrueClass)`
  else
    T.absurd(x)
  end
end
