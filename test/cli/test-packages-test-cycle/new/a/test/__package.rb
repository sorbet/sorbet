# typed: strict
#
# New-style test package, importing the old-style test namespace

class Test::New::A < PackageSpec
  test!

  layer "lib"
  strict_dependencies "false"

  import Test::New::B
end
