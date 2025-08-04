# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::MyPackage < PackageSpec
  layer 'lib'
  strict_dependencies 'layered_dag'
  test_import Foo::Bar::FalseCyclePackageTest
end
