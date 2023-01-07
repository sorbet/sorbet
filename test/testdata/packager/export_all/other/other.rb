# frozen_string_literal: true
# typed: strict

class Other::OtherClass
  Foo::Bar::Thing.hello # This ref still works
  Foo::Bar::OtherThing # anything from the package should be fine
end
