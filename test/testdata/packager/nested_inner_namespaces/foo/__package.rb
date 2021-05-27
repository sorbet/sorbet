# frozen_string_literal: true
# typed: strict
# enable-packager: true

class RootPackage::Foo < PackageSpec
  export RootPackage::Foo::Constant
  export RootPackage::Foo::Bar::Constant
  export RootPackage::Foo::Bar::Baz
end
