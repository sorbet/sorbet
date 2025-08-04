# frozen_string_literal: true
# typed: strict
# enable-packager: true


class Foo::Bar::CyclePackage::SubPackage < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'

  import Foo::Bar::CyclePackage
end
