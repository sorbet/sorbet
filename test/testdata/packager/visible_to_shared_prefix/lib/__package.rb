# typed: strict
# enable-packager: true

class Lib < PackageSpec
  export Lib::X

  visible_to Foo::*
end
