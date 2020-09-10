# typed: true

extend T::Sig

sig { params(blk: T.proc.params(a: Integer, b: String).void).void }
def foo(&blk)
end

def bar
end

foo { T.reveal_type(_1) }
foo do T.reveal_type(_2) end
-> { T.reveal_type(_1) }
[''].each { T.reveal_type(_1) }

bar do
  1 + _2
  puts _1
  puts _2
  T.reveal_type(_2)
  T.reveal_type(_1)
end
