# typed: true

# // Verify `workspace/symbol` can return symbols from standard library, too
# symbol-search: "conjugate", container="::Complex", uri="complex.rbi"
# symbol-search: "conjugate", container="::Float", uri="float.rbi"
# symbol-search: "conjugate", container="::Numeric", uri="numeric.rbi"

# // Multiple symbols from same rbi file?
# symbol-search: "encoding", name="Regexp#encoding", container="::Regexp", uri="regexp.rbi"

# // Extremely common
# symbol-search: "hash", name="Kernel#hash", container="::Kernel", uri="kernel.rbi"
