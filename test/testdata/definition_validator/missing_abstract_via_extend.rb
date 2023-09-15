# typed: true
class Module; include T::Sig; end

module IFoo
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def foo
  end
end

class A # error: Missing definition for abstract method `IFoo#foo`
  extend IFoo
end
