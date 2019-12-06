# typed: true

T::Sig::WithoutRuntime.sig{params(x: Integer, y: T.untyped).returns(T::Boolean)}
def test(x, y)
  x >= y
end

puts(test(10, 10))
puts(test(10, 0))
puts(test(10, 10.0))
puts(test(10, 0.0))
puts(test(10, 100**100))

test(10, "foo")
