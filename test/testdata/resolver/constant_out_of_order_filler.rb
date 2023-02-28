# check-out-of-order-constant-references: true
# typed: true

X = A
#   ^ error: `A` referenced before it is defined

class A::Foo
end
