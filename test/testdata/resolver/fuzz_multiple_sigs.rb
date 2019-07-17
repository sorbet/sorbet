# typed: true
# fuzzed test from https://github.com/sorbet/sorbet/issues/1166
def r
  sig{} # error: Malformed `sig`: No return type specified. Specify one with .returns()
# ^^^^^ error: Method `sig` does not exist on `Object`
# ^^^^^ error: Unused type annotation. No method def before next annotation
  sig{} # error: Malformed `sig`: No return type specified. Specify one with .returns()
# ^^^^^ error: Method `sig` does not exist on `Object`
  def f; end
end
f
