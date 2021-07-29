# frozen_string_literal: true
# typed: true
# compiled: true
# skip_stderr_check

T::Sig::WithoutRuntime.sig{params(x: Integer, y: T.untyped).returns(T::Boolean)}
def test(x, y)
  x != y
end

puts(test(10, 20))
puts(test(10, 10))
puts(test(10, 0))
puts(test(10, 10.0))
puts(test(10, 0.0))
puts(test(10, 20.0))
puts(test(10, -0.0))
puts(test(10, 100**100))
puts(test(10, Float::NAN))
puts(test(10, Float::INFINITY))
puts(test(10, -Float::INFINITY))

# Workaround 2**75 being Numeric according to Sorbet.
big = T.cast(2**75, Integer)
puts(test(big, big + 1))
puts(test(big, big))
puts(test(big, big - 1))
puts(test(big, 0))
puts(test(big, big.to_f + 1))
puts(test(big, big.to_f))
puts(test(big, big.to_f - 1))
puts(test(big, Float::NAN))
puts(test(big, Float::INFINITY))
puts(test(big, -Float::INFINITY))

begin
  test(10, "foo")
rescue ArgumentError => exn
  puts exn.message
end
