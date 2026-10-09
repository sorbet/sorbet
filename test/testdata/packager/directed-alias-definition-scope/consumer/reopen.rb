# typed: true

# Unlike monolithic naming, this stratum sees an already-resolved project alias.
class ::AliasScope::Nested
#     ^^^^^^^^^^^^ error: Defining a root-scoped constant requires this package to be marked `prelude!`
end

  ::AliasTarget::Nested
# ^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `Nested`
