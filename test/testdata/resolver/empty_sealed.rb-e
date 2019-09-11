# typed: true
extend T::Sig

module Bottom
  extend T::Helpers
  sealed!
end

sig {params(x: Bottom).void}
def foo(x)
  case x
  when nil
  else
    T.absurd(x)
  end
end
