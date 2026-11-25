# typed: strict
# frozen_string_literal: true

# stratum: 2

class Parent::MyGeneric # error: File belongs to package `Test::Parent` but defines a constant that does not match this namespace
  Elem2 = type_member
  #       ^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(Parent::MyGeneric)`
end
