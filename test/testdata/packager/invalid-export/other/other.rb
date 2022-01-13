# frozen_string_literal: true
# typed: strict

class Other::OtherClass
  Foo::Bar::Exists.hello # This ref still works
  Foo::Bar::Exists.helloXX
  #                ^^^^^^^ error: Method `helloXX` does not exist on `T.class_of(Foo::Bar::Exists)`


  # Note this is a weird quirk that in this package `NotDefined` exists as a stub. This is safe
  # because an error exists for this in the `__package.rb` file itself.
  Foo::Bar::NotDefined
  Foo::Bar::NotDefined.xxx
  Foo::Bar::NotDefined::Deeper
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `Deeper`
end
