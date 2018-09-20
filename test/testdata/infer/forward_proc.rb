# typed: true
extend T::Helpers

sig(
  blk: T.proc(x: Integer, y: String).returns(Integer)
).returns(Integer)
def f(&blk)
  blk.call(0, "hi")
end

p = T.let(proc {|x, y| x}, T.proc(x: Integer, y: String).returns(Integer))
f(&p)

p = T.let(proc {|x, y| x}, T.proc(x: Integer, y: Integer).returns(Integer))
f(&p) # error: `String` doesn't match `Integer` for argument `arg1`
