# typed: true
# fuzzed test from https://github.com/sorbet/sorbet/issues/1166
def r
  sig{} # error: Method `sig` does not exist on `T.class_of(<root>)`
# ^^^^^ error: Malformed `sig`: No return type specified. Specify one with .returns()
# ^^^^^ error: Unused type annotation. No method def before next annotation
# ^^^^^ error: Malformed `sig`: Signature blocks must contain a single statement
  sig{} # error: Method `sig` does not exist on `T.class_of(<root>)`
# ^^^^^ error: Malformed `sig`: No return type specified. Specify one with .returns()
# ^^^^^ error: Malformed `sig`: Signature blocks must contain a single statement
  def f; end
end
f
