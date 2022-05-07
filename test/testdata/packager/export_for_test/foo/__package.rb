# typed: strict

class Opus::Foo < PackageSpec
  import Opus::Foo::Bar
  import Opus::Util
  test_import Opus::TestImported

  export Opus::Foo::FooClass
  export_for_test Opus::Foo::Private::ImplDetail
# ^^^^^^^^^^^^^^^ error: Method `export_for_test` does not exist
  #               ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Invalid expression in package: Arguments to functions must be literals

  export_for_test Test::Opus::Foo::FooTest
# ^^^^^^^^^^^^^^^ error: Method `export_for_test` does not exist
  #               ^^^^^^^^^^^^^^^^^^^^^^^^ error: Invalid expression in package: Arguments to functions must be literals
end
