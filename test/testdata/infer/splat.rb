# typed: true
extend T::Helpers

sig(x: Integer, y: Integer, z: Integer).returns(Integer)
def f(x, y, z)
  x + y + z
end

args = [1, 2, 3]
T.assert_type!(f(*args), Integer)

T.assert_type!(f(1, *[2, 3]), Integer)

T.reveal_type(f(*T.unsafe(nil))) # error: type: `T.untyped`

sig(x: Integer, blk: T.proc(x: Integer).returns(Integer)).returns(Integer)
def yields(x, &blk)
  blk.call(x)
end

yields(*[1]) do |x|
  T.assert_type!(x, Integer)
  x
end

proc = ->(x){x}
yields(*[1], &T.unsafe(proc))
