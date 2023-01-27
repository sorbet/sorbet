# frozen_string_literal: true
# typed: strict

class Test::Other::OtherClass
  Test::Foo::Bar::Thing.hello # This ref still works
  Test::Foo::Bar::OtherThing # anything from the package should be fine

  Test::Typical::Example # packages that don't use `export_all` are not affected
  Test::Typical::NonExported # packages that don't use `export_all` are not affected
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `Test::Typical::NonExported` resolves but is not exported
end
