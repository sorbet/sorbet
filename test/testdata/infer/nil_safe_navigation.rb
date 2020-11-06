# typed: true

extend T::Sig

sig {params(x: Integer, y: T.nilable(Integer)).void}
def foo(x, y)
  puts x&.to_s       # error: Used `&.` accessor on a receiver which can never be nil
  puts x.to_s

  puts y&.to_s
  puts y.to_s
end
