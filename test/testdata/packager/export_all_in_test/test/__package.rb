# frozen_string_literal: true
# typed: strict

class Test::Foo::Bar < PackageSpec
  test!
  export_all!
  import Foo::Bar, uses_internals: true
  export Test::Foo::Bar::Thing
end
