# typed: true

# Nested blocks with 'it' parameter
# Each block gets its own 'it' scope
# Outer 'it' is Array, inner 'it' is String
[["a", "b"], ["c", "d"]].map { it.map { it.upcase } }
