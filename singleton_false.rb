# typed: strict
extend T::Sig

sig {params(x: T::Boolean).void}
def bar(x)
  case x
  when false
    T.reveal_type(x)
  else
    T.absurd(x)
  end
end
