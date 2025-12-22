# typed: true

# Test that class definition expressions produce correct type for the class symbol.
# The CFG builder emits an Alias instruction for the class symbol in ClassDef expressions.

class A
  class Foo; end
  class Bar; end
  module Baz; end

  # Direct access to class/module symbols produces correct types
  T.reveal_type(Foo) # error: `T.class_of(A::Foo)`
  T.reveal_type(Bar) # error: `T.class_of(A::Bar)`
  T.reveal_type(Baz) # error: `T.class_of(A::Baz)`
end
