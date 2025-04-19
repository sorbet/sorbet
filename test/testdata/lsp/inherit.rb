# typed: true

class Parent
  extend T::Sig

  sig { void }
  def foo; end
end

module IFooBar
  extend T::Sig
  extend T::Helpers
  
  abstract!

  sig { abstract.returns(Integer) }
  def foo; end
end

 class Child < Parent
#^^^^^^^^^^^^^^^^^^^^ error: Return type `void` does not match return type of abstract method `IFooBar#foo`
  include IFooBar
end
