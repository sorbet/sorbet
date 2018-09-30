# typed: true

class Yielder
  extend T::Helpers

  sig {params(blk: T.proc.params(x: Integer, y: Integer).returns(T.untyped)).void}
  def yield_two_ints(&blk); end

  sig {params(blk: T.proc.params(x: Integer, y: Integer, z: Integer).returns(T.untyped)).void}
  def yield_three_ints(&blk); end

  sig {params(blk: T.proc.params(xyz: [Integer, Integer, Integer]).returns(T.untyped)).void}
  def yield_tuple(&blk); end

  sig {params(blk: T.proc.params(xy: [Integer, Integer], z: Integer).returns(T.untyped)).void}
  def yield_tuple_and_int(&blk); end
end

Yielder.new.yield_two_ints do |x, y|
  T.assert_type!(x, Integer)
  T.assert_type!(y, Integer)
end

Yielder.new.yield_two_ints do |x|
  T.assert_type!(x, Integer)
end

Yielder.new.yield_tuple do |x, y, z|
  T.assert_type!(x, Integer)
  T.assert_type!(y, Integer)
  T.assert_type!(z, Integer)
end

Yielder.new.yield_tuple do |(x, y, z)|
  T.assert_type!(x, Integer)
  T.assert_type!(y, Integer)
  T.assert_type!(z, Integer)
end

Yielder.new.yield_tuple do |xyz|
  T.assert_type!(xyz, [Integer, Integer, Integer])
end

Yielder.new.yield_tuple_and_int do |xy, z|
  T.assert_type!(xy, [Integer, Integer])
  T.assert_type!(z, Integer)
end

# Procs with splats
Yielder.new.yield_two_ints do |*x|
  T.assert_type!(x, [Integer, Integer])
end

# Mixing and matching is not yet supported
# Yielder.new.yield_three_ints do |x, *y|
#   T.assert_type!(x, Integer)
#   T.assert_type!(y, [Integer, Integer])
# end

Yielder.new.yield_tuple do |*x|
  T.assert_type!(x, [[Integer, Integer, Integer]])
end

T::Array[T.untyped].new.each do |x, y, z|
  # If a proc is yielded a single T.untyped, it should be willing to
  # expand it arbitrarily wide, under the assumption that it could
  # actually be [T.untyped, T.untyped, T.untyped]

  T.reveal_type(x) # error: type: `T.untyped`
  T.reveal_type(y) # error: type: `T.untyped`
  T.reveal_type(z) # error: type: `T.untyped`
end

T::Array[String].new.each do |x, y, z|
  T.assert_type!(x, String)
  T.assert_type!(y, NilClass)
  T.assert_type!(z, NilClass)
end
