# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::MyPackage < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'
  # import Foo::Bar::OtherPackage ## MISSING!
end
