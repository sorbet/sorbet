# frozen_string_literal: true
# typed: strict
# enable-packager: true

class RootPackage::Foo < PackageSpec
  export Foo
  export Foo::Bar
  export Foo::Bar::Baz
end
