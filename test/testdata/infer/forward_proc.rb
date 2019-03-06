# typed: true
extend T::Sig

sig do
  params(
    blk: T.proc.params(x: Integer, y: String).returns(Integer)
  ).returns(Integer)
end
def f(&blk)
  blk.call(0, "hi")
end

p = T.let(proc {|x, y| x}, T.proc.params(x: Integer, y: String).returns(Integer))
f(&p)

p = T.let(proc {|x, y| x}, T.proc.params(x: Integer, y: Integer).returns(Integer))
f(&p) # error: `T.proc.params(arg0: Integer, arg1: Integer).returns(Integer)` doesn't match `T.proc.params(arg0: Integer, arg1: String).returns(Integer)` for block argument
