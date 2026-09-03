# typed: strict

class OldStyle < PackageSpec
  import Migrated

  test_import Migrated::Test
end
