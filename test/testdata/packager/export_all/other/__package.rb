# frozen_string_literal: true
# typed: strict

class Other < PackageSpec
  import Foo::Bar
  import Foo::Bar::Baz
  import Typical

  export_all!

  export Other::OtherClass
# ^^^^^^^^^^^^^^^^^^^^^^^^ error: Package `Other` declares `export_all!` and therefore should not use explicit exports
  export Other::OtherClass2
# ^^^^^^^^^^^^^^^^^^^^^^^^^ error: Package `Other` declares `export_all!` and therefore should not use explicit exports
end
