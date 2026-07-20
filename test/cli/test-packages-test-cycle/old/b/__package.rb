# typed: strict
#
# Old-style package with an implicit test namespace

class Old::B < PackageSpec
  layer "lib"
  strict_dependencies "dag"

  test_import Old::A
end
