# typed: true

extend T::Sig

sig { params(blk: T.proc.params(a: Integer, b: String).void()).void }
def foo(&blk)
  yield(42, "foo")
end

foo { T.reveal_type(_1) } # error: Revealed type: `Integer`
foo { T.reveal_type(_2) } # error: Revealed type: `String`
foo { T.reveal_type(_3) } # error: Revealed type: `NilClass`
foo { T.reveal_type(_2 * _1) } # error: Revealed type: `String`

foo do
  T.reveal_type(_1) # error: Revealed type: `Integer`
  T.reveal_type(_2) # error: Revealed type: `String`
  T.reveal_type(_3) # error: Revealed type: `NilClass`
  T.reveal_type(_2 * _1) # error: Revealed type: `String`
end

-> { T.reveal_type(_1) } # error: Revealed type: `T.untyped`
-> { T.reveal_type(_9) } # error: Revealed type: `T.untyped`

[1, 2, 3].map { T.reveal_type(_1) } # error: Revealed type: `Integer`

["albatross", "dog", "horse"].max(2) do
  T.reveal_type(_1) # error: Revealed type: `String`
  T.reveal_type(_2) # error: Revealed type: `String`
  _1.length <=> _2.length
end
