# frozen_string_literal: true
# typed: strict

class Test::Foo::Bar::OtherPackage < PackageSpec
  test!

  import Foo::Bar::OtherPackage
end
