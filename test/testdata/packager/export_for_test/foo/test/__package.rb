# typed: strict

class Test::Opus::Foo < PackageSpec
  test!

  import Opus::Foo, uses_internals: true
  import Opus::Foo::Bar
  import Opus::Util
  import Opus::TestImported
  import Test::Opus::Foo::Bar
  import Test::Opus::Util
  import Test::Opus::TestImported
end
