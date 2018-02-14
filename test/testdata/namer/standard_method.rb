# @typed
class T1; end
class T2; end
A1 = T1

class A
  sig(
    a: [T1, T2],
    b: T1,
    c: T.nilable(T1),
    d: T.any(T1, T2),
    e: T.untyped,
    f: T::Array[T1],
    g: T::Hash[T1, T2],
    h: T.enum([false, 1, 3.14, "foo", :bar]),
    i: A1,
    j: T1.singleton_class,
    k: A1.singleton_class,
  )
  .returns(T2)
  def good(a, b, c, d, e, f, g, h, i, j, k)
  end

  sig(
    a: unsupported, # error: Unknown type syntax
    b: T.enum, # error: enum only takes a single argument
    c: T.enum(1),
    d: T.enum([]), # error: enum([]) is invalid
    e: T.enum([meth]), # error: Unsupported type literal
    f: 0, # error: Unsupported type syntax
    g: T.any(*[1,2]),
    h: T.junk, # error: Unsupported method T.junk
  )
  .returns(T2)
  def bad(a, b, c, d, e, f, g, h)
  end

  sig.returns(T.noreturn)
  def noreturn
  end

  sig.returns(T1)
  def no_params
  end

  sig(returns: T1).returns(T2)
  .checked(false)
  def test_kwargs(returns)
  end

  sig(types).returns(T1) # error: Expected a hash of arguments
  def f1(x) # error: Type not specified for argument
  end

  sig(x: T1).returns(T2).hithere # error: Unknown `sig` builder method
  def f2(x)
  end

  sig(x: T1).returns(T1) # error: Unknown argument name x
  def f3
  end

  sig(x: T1).returns(T1)
  private def private(x)
      T1.new
  end

  sig(x: T1).returns(T1)
  protected def protected(x)
      T1.new
  end

  sig(x: T1).returns(T1)
  public def public(x)
      T1.new
  end

  sig(x: T1).returns(T1)
  private_class_method def self.static(x)
      T1.new
  end

  sig(y: T1).returns(T1) # error: Unused type annotation. No method def before next annotation
  sig(y: T1).returns(T1)
  def f4(y)
  end

  sig(x: Integer).returns(Integer)
  def test_yield_no_block_type(x)
    yield x
  end

  sig.abstract.returns(T1)
  def test_abstract; end

  sig.implementation
  def test_implementation(x, y); end

  sig.override.returns(T1)
  def test_override; end

  sig.overridable.returns(T1)
  def test_overridable; end

  sig.implementation.overridable.returns(T1)
  def test_overridable_implementation; end

  sig.abstract; def test_abstract_untyped; end
  sig; def test_standard_untyped; end # error: No return type specified

  sig(z: T1).returns(T1) # error: Malformed sig. No method def following it.
end
