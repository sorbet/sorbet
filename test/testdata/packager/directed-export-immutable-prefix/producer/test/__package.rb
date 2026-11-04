# typed: strict

class Test::Producer < PackageSpec
  test!

  import Producer, uses_internals: true

  # Exporting the alias itself is allowed; traversing it must not export application symbols.
  export Test::Producer::Alias
  export Test::Producer::Alias::Nested # error: Cannot export through class alias
  export Test::Producer::Alias::Value # error: Cannot export through class alias
end
