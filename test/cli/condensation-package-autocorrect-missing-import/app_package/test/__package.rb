# frozen_string_literal: true
# typed: strict

class Test::Foo::Bar::AppPackage < PackageSpec
  test!
  layer 'test'
  strict_dependencies 'false'

  import Foo::Bar::AppPackage
end
