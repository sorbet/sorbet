# typed: strict

class Opus::Foo < PackageSpec
  import Opus::Foo::Bar
  import Opus::Util
  test_import Opus::TestImported

  export Opus::Foo::FooClass
  export_for_test Opus::Foo::Private::ImplDetail
end
