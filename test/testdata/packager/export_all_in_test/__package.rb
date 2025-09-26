# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar < PackageSpec
  export_all!
# ^^^^^^^^^^^ error: Package `Foo::Bar` declares `export_all!` and therefore should not use explicit exports

  export Test::Foo::Bar::Thing

end
