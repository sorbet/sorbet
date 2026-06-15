# frozen_string_literal: true
# typed: strict

class Test::Foo::Bar < PackageSpec
  test!

  import Foo::Bar, uses_internals: true
end
