# typed: strict
require 'sorbet-runtime'
module List
  extend T::Sig
  extend T::Helpers
  extend T::Generic
  include Enumerable
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

    sig do
      override
        .params(blk: T.proc.params(arg0: Elem).returns(BasicObject))
        .returns(T.untyped)
    end
    def each(&blk)
      yield self.head
      self.tail.each(&blk)
    end
  end

  class Nil < T::Struct
    extend T::Sig
    extend T::Generic
    include List
    Elem = type_member
    sig do
      override
        .params(blk: T.proc.params(arg0: Elem).returns(BasicObject))
        .returns(T.untyped)
    end
    def each(&blk)
      nil
    end
  end

  sig {returns(Elem)}
  def head!
    case self
    when List::Nil
      raise ArgumentError.new("Can't get head of empty List")
    when List::Cons
      self.head
    else
      T.absurd(self)
    end
  end

  sig {returns(List[Elem])}
  def tail!
    case self
    when List::Nil
      raise ArgumentError.new("Can't get tail of empty List")
    # Intentionally omitted, to test exhaustiveness errors.
    # when List::Cons
    #   self.tail
    else
      T.absurd(self) # error: the type `List::Cons[List::Elem]` wasn't handled
    end
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
    T.reveal_type(xs) # error: Revealed type: `List::Nil[Integer]`
    # The above error (result type) is because List::Nil does not fix the
    # type_member to T.noreturn, so each empty list can only be used at its
    # own type, instead of being compatible with any list. This is mostly for
    # test coverage; we also have tests that test the fixed noreturn case.
    _unused = T.let(xs, List[String]) # error: Argument does not have asserted type `List[String]`
    List::Nil[String].new
  else
    T.absurd(xs)
  end
end

sig do
  type_parameters(:U, :V)
    .params(
      xs: List[T.type_parameter(:U)],
      f: T.proc.params(arg0: T.type_parameter(:U)).returns(T.type_parameter(:V))
    )
    .returns(List[T.type_parameter(:V)])
end
def list_map_pos(xs, f)
  case xs
  when List::Cons
    head = f.call(xs.head)
    tail = list_map_pos(xs.tail, f)
    List::Cons[T.type_parameter(:V)].new(head: head, tail: tail)
  when List::Nil
    List::Nil[T.type_parameter(:V)].new
  else
    T.absurd(xs)
  end
end

sig do
  type_parameters(:U, :V)
    .params(
      xs: List[T.type_parameter(:U)],
      blk: T.proc.params(arg0: T.type_parameter(:U)).returns(T.type_parameter(:V))
    )
    .returns(List[T.type_parameter(:V)])
end
def list_map_blk(xs, &blk)
  case xs
  when List::Cons
    head = yield xs.head
    tail = list_map_blk(xs.tail, &blk)
    List::Cons[T.type_parameter(:V)].new(head: head, tail: tail)
  when List::Nil
    List::Nil[T.type_parameter(:V)].new
  else
    T.absurd(xs)
  end
end

xs = List::Cons[Integer].new(head: 2, tail: List::Cons[Integer].new(head: 1, tail: List::Nil[Integer].new))
p xs
# Enumerable#map actually creates an Array, because the Enumerable
# interface doesn't include a way to create new instances.
T.reveal_type(xs.map(&:to_s)) # error: Revealed type: `T::Array[String]`

res1 = list_map_pos(xs, -> (x) {
  # Somewhat expected, see https://github.com/sorbet/sorbet/issues/4149
  T.reveal_type(x) # error: Revealed type: `T.untyped`
  x.even?
})
T.reveal_type(res1) # error: Revealed type: `List[T.untyped]`

res2 = list_map_blk(xs) do |x|
  T.reveal_type(x) # error: Revealed type: `Integer`
  x.even?
end
T.reveal_type(res2) # error: Revealed type: `List[T::Boolean]`
