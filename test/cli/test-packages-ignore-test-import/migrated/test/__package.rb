# typed: strict

class Migrated::Test < PackageSpec
  test!

  export Migrated::Test::Helper
  
  import Migrated, uses_internals: true
end
