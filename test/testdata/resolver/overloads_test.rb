# typed: true
# disable-fast-path: true
extend T::Sig

sig {params(s: Integer).void}
sig {params(s: String).void}
def f(s); end

sig {params(s: Symbol).void}
sig {params(s: Float).void}
def f(s); end

f(0) 
f('')
f(:s)  # error: doesn't match `Integer` for argument `s`
f(0.0) # error: doesn't match `Integer` for argument `s`
