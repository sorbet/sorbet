# typed: true

extend T::Sig

module Boolean
  sealed!
end

sig {params(x: Boolean).void}
def foo(x)
  case x
  when TrueClass
    T.reveal_type(x)
  else
    T.absurd(x)
  end
end
