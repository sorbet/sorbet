# typed: true
extend T::Sig

sig {params(x: T.nilable(Integer)).void}
def example1(x)
  x.even?()
  # ^^^^^ error: Method `even?` does not exist on `NilClass` component of `T.nilable(Integer)`
  x.even?(*[])
  # ^^^^^ error: Method `even?` does not exist on `NilClass` component of `T.nilable(Integer)`
end

sig {params(x: Integer).void}
def example2(x)
  x.even()
  # ^^^^ error: Method `even` does not exist on `Integer`
  x.even(*[])
  # ^^^^ error: Method `even` does not exist on `Integer`
end
