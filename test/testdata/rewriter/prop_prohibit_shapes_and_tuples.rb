# typed: true

class A
  # We don't support shape or tuple types for props, and the way that manifests
  # is that the prop rewrite pass just skips these two declarations.
  prop :shape, {a: Integer, b: String}
# ^^^^                                 error: Method `prop` does not exist
  prop :tuple, [Integer, String]
# ^^^^                           error: Method `prop` does not exist
end
