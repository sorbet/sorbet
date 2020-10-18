# typed: true
module List
  extend T::Helpers
  extend T::Generic
  Elem = type_member(:out)
  sealed!

  class Cons
    extend T::Generic
    include List
    Elem = type_member
  end

  class Nil
    extend T::Generic
    include List
    Elem = type_member(fixed: T.noreturn)
  end
end

extend T::Sig

sig do
  type_parameters(:U)
    .params(xs: List[T.type_parameter(:U)])
    .void
end
def foo(xs)
  case xs
  when List::Cons
    T.reveal_type(xs)
  when List::Nil
    T.reveal_type(xs)
  else
    T.absurd(xs)
  end
end

sig do
  type_parameters(:U)
    .params(xs: T.all(List[T.type_parameter(:U)], List::Cons[T.untyped]))
    .void
end
def bar(xs)
  T.reveal_type(xs)
end
