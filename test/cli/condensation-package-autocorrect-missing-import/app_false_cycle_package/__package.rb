# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::AppFalseCyclePackage < PackageSpec
  layer 'app'
  strict_dependencies 'false'

  import Foo::Bar::AppFalseCyclePackage::SubPackage
  import Foo::MyPackage
end
