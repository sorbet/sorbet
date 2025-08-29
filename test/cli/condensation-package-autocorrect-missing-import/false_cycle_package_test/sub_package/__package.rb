# frozen_string_literal: true
# typed: strict
# enable-packager: true


class Foo::Bar::FalseCyclePackageTest::SubPackage < PackageSpec
  layer 'lib'
  strict_dependencies 'false'

  import Foo::Bar::FalseCyclePackageTest
end
