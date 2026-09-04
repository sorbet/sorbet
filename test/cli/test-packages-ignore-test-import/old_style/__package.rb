# typed: strict

class OldStyle < PackageSpec
  import Migrated

  sorbet min_typed_level: "true", tests_min_typed_level: "true"

  test_import Migrated::Test
end
