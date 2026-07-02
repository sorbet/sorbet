# typed: strict
#
# Old-style package with an implicit test namespace

class Old::A < PackageSpec
  layer "lib"
  strict_dependencies "dag"

  test_import Test::New::A
end
