# typed: strict
# stratum: 0

module Parent::Test
  class UsesParent
    Thing = Parent::Thing
  #         ^^^^^^ error: `Parent` is not imported
    Other = Parent::Inner::Other
  #         ^^^^^^ error: `Parent` is not imported
  end
end
