# typed: strict

class Test::InBetween < PackageSpec
  test!

  import InBetween, uses_internals: true
  import Migrated::Test
end
