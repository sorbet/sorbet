# typed: true
extend T::Sig

sig {params(x: T::Types::Base).void}
def foo(x)
  T.nilable(x)
end
