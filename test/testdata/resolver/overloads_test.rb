# typed: true

extend T::Sig

sig {params(s: Integer).void}
sig {params(s: String).void}
def f(s); end

sig {params(s: Symbol).void}
sig {params(s: Float).void}
def f(s); end

f(0) # error: `Integer(0)` doesn't match `Symbol` for argument `s`
f('') # error: `String("")` doesn't match `Symbol` for argument `s`
f(:s)
f(0.0)
