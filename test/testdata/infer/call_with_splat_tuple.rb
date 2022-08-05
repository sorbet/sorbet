# typed: true
extend T::Sig

sig {params(x: T.nilable(Integer)).void}
def example(x)
  x.even?()
  # ^^^^^ error: Method `even?` does not exist on `NilClass` component of `T.nilable(Integer)`
  x.even?(*[])
end
