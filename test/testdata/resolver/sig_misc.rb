# typed: true
class T1; end
class T2; end

class A
  extend T::Sig
  extend T::Helpers
  sig {returns(T.noreturn)}
  def noreturn
    raise "foo"
  end

  sig {returns(T1)}
  def no_params
    T1.new
  end

  sig do
    params(returns: T1).returns(T2)
    .checked(false)
  end
  def test_kwargs(returns)
    T2.new
  end

  sig {params(types).returns(T1)}
            # ^^^^^ error: Method `types` does not exist
     # ^^^^^^^^^^^^^ error: `params` expects keyword arguments
  def f1(x) # error: Type not specified for argument
    T1.new
  end

  sig do params(x: T1).returns(T2) end.hithere # error: Can not call method `hithere` on void type
  def f2(x)
    T2.new
  end

  sig {params(x: T1).returns(T1)} # error: Unknown argument name `x`
  def f3
    T1.new
  end

  sig {params(x: T1).returns(T1)}
  private def private(x)
    T1.new
  end

  sig {params(x: T1).returns(T1)}
  protected def protected(x)
    T1.new
  end

  sig {params(x: T1).returns(T1)}
  public def public(x)
    T1.new
  end

  sig {params(x: T1).returns(T1)}
  private_class_method def self.static(x)
    T1.new
  end

  sig {params(y: T1).returns(T1)} # error: Unused type annotation. No method def before next annotation
  sig {params(y: T1).returns(T1)}
  def f4(y)
    T1.new
  end

  sig {params(x: Integer).returns(Integer)}
  def test_yield_no_block_type(x)
    yield x
  end

  abstract!

  sig {abstract.returns(T1)}
  def test_abstract;
   # empty, because abstract
  end

  sig { override.void }
  def test_implementation(x) # error: Method `A#test_implementation` is marked `override` but does not override anything
                        # ^ error: Malformed `sig`. Type not specified for argument
  end

  sig {override.returns(T1)}
  def test_override; # error: Method `A#test_override` is marked `override` but does not override anything
    T1.new
  end

  sig {overridable.returns(T1)}
  def test_overridable;
    T1.new
  end

  sig {override.overridable.returns(T1)}
  def test_overridable_implementation; # error: Method `A#test_overridable_implementation` is marked `override` but does not override anything
    T1.new
  end

  sig {abstract} # error: Malformed `sig`: No return type specified. Specify one with .returns()
  def test_abstract_untyped
  end

  sig {}; def test_standard_untyped; end # error: Malformed `sig`: No return type specified. Specify one with .returns()

  sig {void.foo}; def test_junk_inside; end # error: invalid in this context
     # ^^^^^^^^ error: Method `foo` does not exist

  sig {T.void}; def test_junk_again; end # error: being invoked on an invalid receiver
     # ^^^^^^ error: Method `void` does not exist

  sig {params(z: T1).returns(T1)} # error: Malformed `sig`. No method def following it
end
