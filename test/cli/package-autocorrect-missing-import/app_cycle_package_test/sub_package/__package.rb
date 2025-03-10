# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar::AppCyclePackageTest::SubPackage < PackageSpec
  layer 'app'
  strict_dependencies 'layered'
  import Foo::Bar::AppCyclePackageTest
end
