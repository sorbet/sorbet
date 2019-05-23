# typed: strict
class A < B # error: Circular dependency: `A` is a parent of itself
end
B = A
