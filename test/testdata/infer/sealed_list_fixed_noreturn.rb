# typed: strict
module List
  extend T::Sig
  extend T::Helpers
  extend T::Generic
  Elem = type_member(:out)
  sealed!
  abstract!

  sig {abstract.returns(Elem)}
  def head; end

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
    Elem = type_member {{fixed: T.noreturn}}

    sig {override.returns(Elem)}
    def head; raise "head on empty list"; end
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
    T.reveal_type(xs) # error: Revealed type: `List::Nil`

    _unused = T.let(xs, List[String])
    0.times do
      puts(xs.head) # error: This code is unreachable
    end
    xs
  else
    T.absurd(xs)
  end
end
