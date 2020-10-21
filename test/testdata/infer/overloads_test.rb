# typed: true
# disable-stress-incremental: true

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
      _: String,
    )
    .returns(String)
  end
  sig do
    params(
        _: Exception,
      )
    .returns(NilClass)
  end
  sig do
    params(
      _: Class,
      _1: String,
      _2: T::Array[String],
    )
    .returns(Symbol)
  end
  def overloaded(_, _1=_, _2=_);
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

  def _; end

  sig {params(x: Elem).returns(Elem)}
  sig {params(x: String).returns(String)}
  def overloaded(x); _; end
end

class Foo
  def test
    h = HasOverloads.new
    T.assert_type!(h.overloaded(), Integer)
    T.assert_type!(h.overloaded("s"), String)
    T.assert_type!(h.overloaded(Exception.new), NilClass)
    T.assert_type!(h.overloaded(self.class), Symbol)
    h.overloaded(1) # error: Expected `String` but found `Integer(1)` for argument `_`
                    # should ask for string
    h.overloaded("1", 2) # error: Expected `Class` but found `String("1")` for argument `_`
  #                   ^ error: Expected `String` but found `Integer(2)` for argument `_1`

    g = OverloadAndGenerics[Integer].new
    T.assert_type!(g.overloaded("hi"), String)
    T.assert_type!(g.overloaded(4), Integer)
  end
end
