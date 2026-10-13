# typed: true

# Unlike monolithic naming, this stratum sees an already-resolved project alias.
class ::AliasScope::Nested # error: Defining a root-scoped constant requires this package to be marked `prelude!`
#     ^^^^^^^^^^^^ error: Redefining constant `AliasScope` as a class or module

end

  ::AliasTarget::Nested
# ^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `Nested`
