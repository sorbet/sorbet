# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar < PackageSpec

  export Foo::Bar::Exists
  export Foo::Bar::NotDefined
#        ^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `NotDefined`
end
