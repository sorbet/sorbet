# typed: strict

class InBetween < PackageSpec
  sorbet min_typed_level: "true", tests_min_typed_level: "true"

  import Migrated

  test_import Migrated::Test
end
