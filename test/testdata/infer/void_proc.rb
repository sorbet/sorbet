# typed: true

extend T::Sig

sig {params(blk: T.proc.void).void}
def foo(&blk)
end

foo do
  3
end

sig {params(f: T.proc.void).void}
def takes_void_proc(f)
end

f = T.let(-> () { 0 }, T.proc.returns(Integer))
takes_void_proc(f)
