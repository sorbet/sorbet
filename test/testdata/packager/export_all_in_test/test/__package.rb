# frozen_string_literal: true
# typed: strict

class Test::Foo::Bar < PackageSpec
  test!
  export_all!
# ^^^^^^^^^^^ error: Package `Test::Foo::Bar` declares `export_all!` and therefore should not use explicit exports
  import Foo::Bar, uses_internals: true
  export Test::Foo::Bar::Thing
end
