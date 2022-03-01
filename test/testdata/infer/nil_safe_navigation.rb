# typed: true

extend T::Sig

sig {params(x: Integer, y: T.nilable(Integer)).void}
def foo(x, y)
  puts x&.to_s
  #     ^^ error: Used `&.` operator on `Integer`, which can never be nil
  puts x.to_s

  puts y&.to_s
  puts y.to_s
end
