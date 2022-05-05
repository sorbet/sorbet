# frozen_string_literal: true
# typed: strict

class Other::OtherClass
  Foo::Bar::Exists.hello # This ref still works
  Foo::Bar::Exists.helloXX
  #                ^^^^^^^ error: Method `helloXX` does not exist on `T.class_of(Foo::Bar::Exists)`


  Foo::Bar::NotDefined
# ^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `NotDefined`
  Foo::Bar::NotDefined.xxx
# ^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `NotDefined`
  Foo::Bar::NotDefined::Deeper
# ^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve constant `NotDefined`
end
