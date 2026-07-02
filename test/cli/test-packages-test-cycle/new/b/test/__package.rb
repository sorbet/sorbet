# typed: strict
#
# New-style test package, importing the old-style test namespace

class Test::New::B < PackageSpec
  test!

  layer "lib"
  strict_dependencies "false"

  import Test::New::A
end
