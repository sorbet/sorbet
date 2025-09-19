# frozen_string_literal: true
# typed: strict
# enable-packager: true

class Foo::Bar < PackageSpec

  export_for_test Foo::Bar::Exists
# ^^^^^^^^^^^^^^^ error: Method `export_for_test` does not exist
#                 ^^^^^^^^^^^^^^^^ error: Invalid expression in package: Arguments to functions must be literals
  export Foo::Bar::Exists

  export Foo::Bar::Also
  export Foo::Bar::Also
# ^^^^^^^^^^^^^^^^^^^^^ error: Duplicate export of `Foo::Bar::Also`
end
