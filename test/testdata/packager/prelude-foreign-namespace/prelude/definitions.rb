# typed: strict

# Reopening a foreign namespace is currently isolated without a diagnostic.
module ::Owner
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
