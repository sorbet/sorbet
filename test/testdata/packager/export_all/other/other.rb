# frozen_string_literal: true
# typed: strict

class Other::OtherClass
  Foo::Bar::Thing.hello # This ref still works
  Foo::Bar::OtherThing # anything from the package should be fine

  # because `Foo::Bar::Baz` is a subpackage, the export walk should stop
  # and not export the stuff under that namespace
  Foo::Bar::Baz::Quux
# ^^^^^^^^^^^^^^^^^^^ error: `Foo::Bar::Baz::Quux` resolves but is not exported

  Typical::Example # packages that don't use `export_all` are not affected
  Typical::NonExported # packages that don't use `export_all` are not affected
# ^^^^^^^^^^^^^^^^^^^^ error: `Typical::NonExported` resolves but is not exported
end
