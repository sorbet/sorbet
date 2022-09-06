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

def example4
  xs = []
  xs += "asdf"
  #     ^^^^^^ error: Expected `T::Enumerable[T.untyped]` but found `String("asdf")` for argument `arg0`
  T.reveal_type(xs) # error: `T::Array[T.untyped]`
end

def example5
  xs = []
  xs += {}
  T.reveal_type(xs) # error: `T::Array[T.untyped]`
end

sig {params(ys: T::Array[T.untyped]).void}
def example6(ys)
  xs = []
  xs += ys
  T.reveal_type(xs) # error: `T::Array[T.untyped]`
end
