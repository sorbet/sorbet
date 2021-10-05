# typed: strict

class Opus::Foo < PackageSpec
  import Opus::Foo::Bar
  import Opus::Util
  test_import Opus::TestImported

  export Opus::Foo::FooClass
  export_for_test Opus::Foo::Private::ImplDetail

  export_for_test Test::Opus::Foo::FooTest
  #               ^^^^^^^^^^^^^^^^^^^^^^^^ error: Packages may not export_for_test names in the `Test::` namespace
end
