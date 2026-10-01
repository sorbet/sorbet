# typed: strict
# frozen_string_literal: true

class Parent::MyGeneric # error: Tests in the `Parent` package must define tests in the `Test::Parent` namespace
  Elem2 = type_member
  #       ^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(Parent::MyGeneric)`
end
