# typed: strict
extend T::Sig

module List
  extend T::Sig
  extend T::Generic
  abstract!

  Elem = type_member(:out)

  sig {abstract.returns(Elem)}
  def head!; end
end

class Nil
  extend T::Sig
  extend T::Generic
  include List

  Elem = type_member {{fixed: T.noreturn}}

  sig {override.returns(Elem)}
  def head!
    raise "Called head on Nil list"
  end

  sig {void}
  def example
    head!.not_exist # error: This code is unreachable
  end
end

class Cons < T::Struct
  extend T::Sig
  extend T::Generic

  include List

  Elem = type_member

  const :head, Elem, override: true
  const :tail, List[Elem], override: true

  sig {override.returns(Elem)}
  def head!; head; end
end

sig {params(xs: Nil).void}
def takes_nil(xs)
  xs.head!.not_exists # error: This code is unreachable
end

T.reveal_type(Nil.new) # error: `Nil`
empty = T.let(Nil.new, List[Integer])
T.let(Cons.new(head: 0, tail: empty), List[Integer])
T.let(Cons.new(head: 0, tail: Nil.new), List[Integer])
