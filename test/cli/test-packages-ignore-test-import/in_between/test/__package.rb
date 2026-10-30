# typed: strict

class Test::InBetween < PackageSpec
  test!

  sorbet min_typed_level: "true"

  import InBetween, uses_internals: true
  import Migrated::Test
end
