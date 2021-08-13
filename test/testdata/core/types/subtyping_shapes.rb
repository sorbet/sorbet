# typed: true
extend T::Sig

sig { params(x: T.any({a: Integer}, [String])).void }
def bar(x)
  T.reveal_type(x) # error: Revealed type: `T.any({a: Integer}, [String])`
end

x = T.let(["b"], T.any([String], {a: Integer}))
T.reveal_type(x) # error: Revealed type: `T.any({a: Integer}, [String])`

bar({a: 1})
bar(["ok"])
bar({a: "bad"}) # error: Expected `T.any({a: Integer}, [String])` but found `{a: String("bad")}` for argument `x`
bar({b: -1}) # error: Expected `T.any({a: Integer}, [String])` but found `{b: Integer(-1)}` for argument `x`
bar({c: -1}) # error: Expected `T.any({a: Integer}, [String])` but found `{c: Integer(-1)}` for argument `x`
