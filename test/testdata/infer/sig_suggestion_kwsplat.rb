# typed: strict
extend T::Sig

sig { params(a: Integer, b: Integer, c: Float, x: String).void }
def callee(a, b = 0, c = 0.0, x: ''); end

# The keyword arg passed alongside the splat has to be looked up by name. Reading the
# arguments positionally instead would suggest `c`'s type for `q` rather than `x`'s.
def suggest_with_kwsplat(p, q, opts) # error: does not have a `sig`
  callee(p, x: q, **opts)
end

def suggest_without_kwsplat(p, q) # error: does not have a `sig`
  callee(p, x: q)
end

def suggest_with_lone_kwsplat(p, opts) # error: does not have a `sig`
  callee(p, **opts)
end
