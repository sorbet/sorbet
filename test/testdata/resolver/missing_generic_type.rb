# typed: __STDLIB_INTERNAL
# disable-fast-path: true

class A # error: Type `Elem` declared by parent `Enumerable` must be re-declared in `A`
# error: Missing definition for abstract method `Enumerable#each`
  include Enumerable
end

class B < A # error: Missing definition for abstract method `Enumerable#each`
  include Enumerable
  extend T::Generic
  Elem = type_member(fixed: T.untyped)
end
