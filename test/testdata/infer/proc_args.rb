# typed: true

class Yielder
  sig(blk: T.proc(x: Integer, y: Integer).returns(T.untyped)).void
  def yield_two_ints(&blk); end

  sig(blk: T.proc(xyz: [Integer, Integer, Integer]).returns(T.untyped)).void
  def yield_tuple(&blk); end

  sig(blk: T.proc(xy: [Integer, Integer], z: Integer).returns(T.untyped)).void
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
