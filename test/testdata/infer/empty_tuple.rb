# typed: true
extend T::Sig

sig {params(xs: T::Array[Integer]).void}
def test1(xs = [])
  T.reveal_type(xs) # error: Revealed type: `T::Array[Integer]`
end

sig {params(x: T::Array[String]).void}
def test2(x)
  x = []
  T.reveal_type(x) # error: Revealed type: `[] (0-tuple)`

  if T.unsafe(nil)
    y = []
    z = T::Array[Integer].new
  else
    y = T::Array[Integer].new
    z = []
  end
  T.reveal_type(y) # error: Revealed type: `T::Array[Integer]`
  T.reveal_type(z) # error: Revealed type: `T::Array[Integer]`
end
