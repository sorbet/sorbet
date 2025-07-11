# frozen_string_literal: true
# typed: strict
# enable-packager: true


class Foo::Bar::FalseCyclePackage::SubPackage < PackageSpec
  layer 'lib'
  strict_dependencies 'false'

  import Foo::Bar::FalseCyclePackage
end
