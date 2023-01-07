# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar < PackageSpec
  export_all!

  export Foo::Bar::Thing
       # ^^^^^^^^^^^^^^^ error: Package `Foo::Bar` declares `export_all!` and therefore should not use explicit exports

end
