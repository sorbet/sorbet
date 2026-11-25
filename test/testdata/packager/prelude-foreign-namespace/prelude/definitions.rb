# typed: strict

# Explicit-root preludes cannot reopen another package's namespace.
module ::Owner # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
  class Nested; end
  VALUE = 1
end





# Explicit-root prelude definitions of unpackaged namespaces remain valid.
module ::UnpackagedPrelude
  class Nested; end
  VALUE = 1
end

class ::String
  class PreludePatch; end
end

# A prelude package can also define its own namespace explicitly.
module ::Prelude
  class Own; end
  VALUE = 1
end
