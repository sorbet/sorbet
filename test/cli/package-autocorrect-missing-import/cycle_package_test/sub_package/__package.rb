# frozen_string_literal: true
# typed: strict
# enable-packager: true


class Foo::Bar::CyclePackageTest::SubPackage < PackageSpec
  layer 'lib'
  strict_dependencies 'layered'

  import Foo::Bar::CyclePackageTest
end
