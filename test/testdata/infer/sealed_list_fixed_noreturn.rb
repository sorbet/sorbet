# typed: strict
module List
  extend T::Sig
  extend T::Helpers
  extend T::Generic
  Elem = type_member(:out)
  sealed!
  abstract!

  class Cons < T::Struct
    extend T::Sig
    extend T::Generic
    include List
    Elem = type_member

    prop :head, Elem
    prop :tail, List[Elem]
  end

  class Nil < T::Struct
    extend T::Sig
    extend T::Generic
    include List
    Elem = type_member(fixed: T.noreturn)
  end
end

extend T::Sig

sig do
  type_parameters(:U)
    .params(xs: List[Integer])
    .returns(List[String])
end
def list_integer_to_list_string(xs)
  case xs
  when List::Cons
    List::Cons[String].new(head: xs.head.to_s, tail: list_integer_to_list_string(xs))
  when List::Nil
    T.reveal_type(xs) # error: Revealed type: `T.all(List[Integer], List::Nil)`

    # Even though the T.all doesn't collapse above, it appears that type can
    # still be used for any empty list?  Not sure if there's a bug in
    # Types::glb, or in Types::isSubType, or if there's no bug.
    _unused = T.let(xs, List[String])
    xs
  else
    T.absurd(xs)
  end
end
