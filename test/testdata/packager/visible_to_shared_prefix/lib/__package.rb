# typed: strict
# enable-packager: true

class Lib < PackageSpec
  visible_to Foo::*
end
