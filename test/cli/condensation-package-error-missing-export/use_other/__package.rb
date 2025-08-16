# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::MyPackage < PackageSpec
  import Foo::Bar::OtherPackage
end
