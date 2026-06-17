# frozen_string_literal: true
# typed: strict

class Test::Foo::MyPackage < PackageSpec
  test!

  import Foo::MyPackage
end
