# typed: true
extend T::Sig

sig { params(x: T::Hash[Symbol, Numeric]).void }
def foo(x); end

sig { params(x: T::Hash[Symbol, T.nilable(Numeric)]).void }
def bar(x); end

class Baz; end

sig { params(x: T::Hash[Symbol, T.nilable(Baz)]).void }
def baz(x); end

xi = {a: 1, b: 2}
T.reveal_type(xi) # error: Revealed type: `{a: Integer(1), b: Integer(2)} (shape of T::Hash[Symbol, Integer])`
foo(xi)
bar(xi)

xf = {a: 1.0, b: 2.0}
T.reveal_type(xf) # error: Revealed type: `{a: Float(1.000000), b: Float(2.000000)} (shape of T::Hash[Symbol, Float])`
foo(xf)
bar(xf)

xni = {a: 1, b: nil}
T.reveal_type(xni) # error: Revealed type: `{a: Integer(1), b: NilClass} (shape of T::Hash[Symbol, T.nilable(Integer)])`
bar(xni)
foo(xni) # error: Expected `T::Hash[Symbol, Numeric]` but found `{a: Integer(1), b: NilClass}` for argument `x`

xif = {a: 1, b: 2.0}
T.reveal_type(xif) # error: Revealed type: `{a: Integer(1), b: Float(2.000000)} (shape of T::Hash[Symbol, T.untyped])`
foo(xif)

T.reveal_type({a: 1, "b" => 2}) # error: Revealed type: `{a: Integer(1), String("b") => Integer(2)} (shape of T::Hash[T.any(Symbol, String), Integer])`
T.reveal_type({a: 1, "b" => 2.0}) # error: Revealed type: `{a: Integer(1), String("b") => Float(2.000000)} (shape of T::Hash[T.any(Symbol, String), T.untyped])`
T.reveal_type({a: 1, "b" => 2.0, 3 => nil}) # error: Revealed type: `{a: Integer(1), String("b") => Float(2.000000), Integer(3) => NilClass} (shape of T::Hash[T.any(Symbol, String, Integer), T.untyped])`
T.reveal_type({a: 1.0, "b" => 2.0, 3 => nil}) # error: Revealed type: `{a: Float(1.000000), String("b") => Float(2.000000), Integer(3) => NilClass} (shape of T::Hash[T.any(Symbol, String, Integer), T.nilable(Float)])`

xz = {a: Baz.new, b: Baz.new}
T.reveal_type(xz) # error: Revealed type: `{a: Baz, b: Baz} (shape of T::Hash[Symbol, Baz])`
baz(xz)

xnz = {a: nil, b: Baz.new}
T.reveal_type(xnz) # error: Revealed type: `{a: NilClass, b: Baz} (shape of T::Hash[Symbol, T.nilable(Baz)])`
baz(xnz)
