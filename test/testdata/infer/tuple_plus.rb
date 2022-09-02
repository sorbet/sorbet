# typed: true
extend T::Sig

def example1
  xs = []
  T.reveal_type(xs) # error: `[] (0-tuple)`
  xs += [1]
  T.reveal_type(xs) # error: `[Integer(1)] (1-tuple)`
  xs += [2]
  T.reveal_type(xs) # error: `T::Array[Integer]`
end

sig {params(ys: T::Array[Integer]).void}
def example2(ys)
  xs = []
  T.reveal_type(xs) # error: `[] (0-tuple)`
  xs += ys
  T.reveal_type(xs) # error: `T::Array[Integer]`
end

sig {params(ys: T::Array[Integer]).void}
def example3(ys)
  xs = []
  T.reveal_type(xs) # error: `[] (0-tuple)`
  ys += xs
  T.reveal_type(ys) # error: `T::Array[Integer]`
end
