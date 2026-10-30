# typed: strict

# Defining `A::B` creates `A` in this package's stratum, which is before `A`'s own. `A`'s stratum must still
# be able to mark `A` exported (it's the parent namespace of the exported `A::Thing`).
module A::B
  class Service; end
end
