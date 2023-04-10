# check-out-of-order-constant-references: true
# typed: true

X = A # No error. We do not report undeclared (implicit definition) symbols.

class A::Foo
end
