# typed: strict
extend T::Sig

module IFoo
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.returns(String)}
  private def foo_impl; end

  sig {abstract.returns(String)}
  def foo; end
end

module IFooDefault
  extend T::Sig
  extend T::Helpers
  include IFoo
  abstract!

  sig {override.returns(String)}
  def foo; foo_impl; end # good
end


class FooWithDefault
  extend T::Sig
  include IFooDefault

  sig {override.returns(String)}
  def foo_impl; 'FooWithDefault'; end
end

class FooCustom
  extend T::Sig
  include IFoo

  sig {override.returns(String)}
  def foo_impl; 'FooCustom'; end

  sig {override.returns(String)}
  def foo
    res = foo_impl # good
    puts("Custom foo: #{res}")
    res
  end
end

sig {params(x: IFoo).void}
def example(x)
  x.foo      # good
  x.foo_impl # error: Non-private call to private method `foo_impl` on `IFoo`
end
