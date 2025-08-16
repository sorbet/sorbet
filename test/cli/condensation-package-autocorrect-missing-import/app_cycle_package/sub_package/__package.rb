# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::AppCyclePackage::SubPackage < PackageSpec
  layer 'app'
  strict_dependencies 'layered'
end