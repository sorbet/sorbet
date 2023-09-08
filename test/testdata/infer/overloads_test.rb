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
  def overloaded(arg0, arg1=arg0, arg2=arg0);
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
  def invalid_overloaded(a:, b:);
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
  def overloaded(x); arg0; end
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
  private def foo(x=nil); end
end

po1 = PrivateOverloads.new.foo # error: Non-private call to private method
T.reveal_type(po1) # error: Revealed type: `NilClass`

po2 = PrivateOverloads.new.foo(0) # error: Non-private call to private method
T.reveal_type(po2) # error: Revealed type: `Integer`
