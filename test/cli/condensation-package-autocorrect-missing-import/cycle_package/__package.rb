# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::CyclePackage < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'

  import Foo::MyPackage
  import Foo::Bar::CyclePackage::SubPackage
end
