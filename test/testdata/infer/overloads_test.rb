# typed: true

# MutableContext::permitOverloadDefinitions is aware of this file name. Don't rename this file.
class HasOverloads
  extend T::Sig

  sig do
    returns(T.untyped())
  end
  def make_untyped
  end

  sig do
    returns(Integer)
  end
  sig do
    params(
      arg0: String,
    )
    .returns(String)
  end
  sig do
    params(
        arg0: Exception,
      )
    .returns(NilClass)
  end
  sig do
    params(
      arg0: Class,
      arg1: String,
      arg2: T::Array[String],
    )
    .returns(Symbol)
  end
  def overloaded(arg0, arg1=arg0, arg2=arg0); # error: against an overloaded signature
    make_untyped
  end

  sig do
    params(
      a: Class,  # error: Malformed `sig`
      b: String, # error: Malformed `sig`
    )
    .returns(Integer)
  end
  sig do
    params(
      b: Class,  # error: Malformed `sig`
      a: String, # error: Malformed `sig`
    )
    .returns(Symbol)
  end
  def invalid_overloaded(a:, b:); # error: against an overloaded signature
    #                    ^^ error: Bad parameter ordering for `a`, expected `b` instead
    #                        ^^ error: Bad parameter ordering for `b`, expected `a` instead
    make_untyped
  end
end

class OverloadAndGenerics
  extend T::Generic
  extend T::Sig
  Elem = type_member

  def arg0; end

  sig {params(x: Elem).returns(Elem)}
  sig {params(x: String).returns(String)}
  def overloaded(x); arg0; end # error: against an overloaded signature
end

class Foo
  def test
    h = HasOverloads.new
    T.assert_type!(h.overloaded(), Integer)
    T.assert_type!(h.overloaded("s"), String)
    T.assert_type!(h.overloaded(Exception.new), NilClass)
    T.assert_type!(h.overloaded(self.class), Symbol)
    h.overloaded(1) # error: Expected `String` but found `Integer(1)` for argument `arg0`
                    # should ask for string
    h.overloaded("1", 2) # error: Expected `T::Class[T.anything]` but found `String("1")` for argument `arg0`
  #                   ^ error: Expected `String` but found `Integer(2)` for argument `arg1`

    g = OverloadAndGenerics[Integer].new
    T.assert_type!(g.overloaded("hi"), String)
    T.assert_type!(g.overloaded(4), Integer)
  end
end

class PrivateOverloads
  extend T::Sig

  sig {returns(NilClass)}
  sig {params(x: Integer).returns(Integer)}
  private def foo(x=nil); end # error: against an overloaded signature
end

po1 = PrivateOverloads.new.foo # error: Non-private call to private method
T.reveal_type(po1) # error: Revealed type: `NilClass`

po2 = PrivateOverloads.new.foo(0) # error: Non-private call to private method
T.reveal_type(po2) # error: Revealed type: `Integer`


class A; end
class B; end

class BlockOverloads
  extend T::Sig

  sig { returns(A) }
  sig { params(blk: T.proc.void).returns(B) }
  def simple(&blk); end # error: against an overloaded signature

  sig { params(blk: NilClass).returns(A) }
  sig { params(blk: T.proc.void).returns(B) }
  def explicit_nilclass(&blk); end # error: against an overloaded signature

  sig { returns(A) }
  sig { params(blk: T.nilable(T.proc.void)).returns(B) }
  def ambiguous_nilable_a(&blk); end # error: against an overloaded signature
  sig { params(blk: T.nilable(T.proc.void)).returns(B) }
  sig { returns(A) }
  def ambiguous_nilable_b(&blk); end # error: against an overloaded signature

  sig { returns(A) }
  sig { params(blk: T.untyped).returns(B) }
  def ambiguous_untyped_a(&blk); end # error: against an overloaded signature
  sig { params(blk: T.untyped).returns(B) }
  sig { returns(A) }
  def ambiguous_untyped_b(&blk); end # error: against an overloaded signature

  sig { returns(A) }
  sig do
    type_parameters(:U)
      .params(blk: T.proc.returns(T.type_parameter(:U)))
      .returns(T.type_parameter(:U))
  end
  def not_fully_defined(&blk); end # error: against an overloaded signature
  sig do
    type_parameters(:U)
      .params(blk: T.proc.returns(T.type_parameter(:U)))
      .returns(T.type_parameter(:U))
  end
  sig { returns(A) }
  def not_fully_defined_flipped(&blk); end # error: against an overloaded signature

  def test
    x = simple
    T.reveal_type(x) # error: `A`
    x = simple {}
    T.reveal_type(x) # error: `B`
    x = simple(&:to_s)
    T.reveal_type(x) # error: `B`


    x = explicit_nilclass
    T.reveal_type(x) # error: `A`
    x = explicit_nilclass {}
    T.reveal_type(x) # error: `B`


    x = ambiguous_nilable_a
    T.reveal_type(x) # error: `A`
    x = ambiguous_nilable_a {}
    T.reveal_type(x) # error: `B`

    x = ambiguous_nilable_b
    T.reveal_type(x) # error: `B`
    x = ambiguous_nilable_b {}
    T.reveal_type(x) # error: `B`


    x = ambiguous_untyped_a
    T.reveal_type(x) # error: `A`
    x = ambiguous_untyped_a {}
    T.reveal_type(x) # error: `B`

    x = ambiguous_untyped_b
    T.reveal_type(x) # error: `B`
    x = ambiguous_untyped_b {}
    T.reveal_type(x) # error: `B`


    x = not_fully_defined
    T.reveal_type(x) # error: `A`
    x = not_fully_defined {B.new}
    T.reveal_type(x) # error: `B`

    x = not_fully_defined_flipped
    T.reveal_type(x) # error: `A`
    x = not_fully_defined_flipped {B.new}
    T.reveal_type(x) # error: `B`
  end
end
