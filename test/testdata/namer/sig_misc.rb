# @typed
class T1; end
class T2; end

class A
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

  sig(x: T1).returns(T1) # error: Unknown argument name `x`
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
