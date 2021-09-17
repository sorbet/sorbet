# typed: true
extend T::Sig

sig { params(x: T.any({a: Integer}, {b: String})).void }
def foo(x)
  T.reveal_type(x) # error: Revealed type: `T.any({a: Integer}, {b: String})`
end

foo({a: 1})
foo({b: "ok"})
foo({a: "bad"}) # error: Expected `T.any({a: Integer}, {b: String})` but found `{a: String("bad")}` for argument `x`
foo({b: -1}) # error: Expected `T.any({a: Integer}, {b: String})` but found `{b: Integer(-1)}` for argument `x`
foo({c: -1}) # error: Expected `T.any({a: Integer}, {b: String})` but found `{c: Integer(-1)}` for argument `x`

sig { params(x: T.any({a: Integer, b: String}, {a: Numeric, b: Object}, {b: String})).void }
def bar(x)
  T.reveal_type(x) # error: Revealed type: `T.any({a: Numeric, b: Object}, {b: String})`
end

bar({a: 1, b: "too"})
bar({a: 1.5, b: Object.new})
bar({b: "too"})
bar({b: Object.new}) # error: Expected `T.any({a: Numeric, b: Object}, {b: String})` but found `{b: Object}` for argument `x`

sig { params(x: T.any({a: Integer}, [String])).void }
def baz(x)
  T.reveal_type(x) # error: Revealed type: `T.any({a: Integer}, [String])`
end

x = T.let(["b"], T.any([String], {a: Integer}))
T.reveal_type(x) # error: Revealed type: `T.any({a: Integer}, [String])`

baz({a: 1})
baz(["ok"])
baz({a: "bad"}) # error: Expected `T.any({a: Integer}, [String])` but found `{a: String("bad")}` for argument `x`
baz({b: -1}) # error: Expected `T.any({a: Integer}, [String])` but found `{b: Integer(-1)}` for argument `x`
baz({c: -1}) # error: Expected `T.any({a: Integer}, [String])` but found `{c: Integer(-1)}` for argument `x`
