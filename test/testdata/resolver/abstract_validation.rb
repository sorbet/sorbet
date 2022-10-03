# typed: true
# Fast path causes duplicate errors.

module AbstractMixin
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.returns(Object)}
  def foo; end

  sig {abstract.returns(Object)}
  def bar; end

  sig {returns(Object)}
  def concrete_standard; end

  def concrete_no_signature; end
end


class AbstractClass
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.returns(Object)}
  def self.foo; end

  sig {abstract.returns(Object)}
  def bar; end
end

class ImplEnabling < AbstractClass
  sig {override(allow_incompatible: false).params(x: Integer).returns(Object)}
  def self.foo(x); end # error: `AbstractClass.foo` must accept no more than `0` required argument(s)

  sig {override(allow_incompatible: true).params(x: Integer).returns(Object)}
  def bar(x); end
end


# it raises an error when defining an abstract class method on a
# module
module M1
  extend T::Sig
  extend T::Helpers
  abstract!
  sig {abstract.returns(Object)}
  def self.foo; end # error: Static methods in a module cannot be abstract
end

# it fails if a concrete module doesn't implement abstract methods
  module M2
# ^^^^^^^^^ error: Missing definition for abstract method `AbstractMixin#bar`
# ^^^^^^^^^ error: Missing definition for abstract method `AbstractMixin#foo`
  extend T::Helpers
  include AbstractMixin
end

# it fails if a class method is unimplemented
  class C3 < AbstractClass
# ^^^^^^^^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method `AbstractClass.foo`
  extend T::Sig
  extend T::Helpers
  sig { override.returns(Object) }
  def bar; end
end

# it fails if an instance method is unimplemented
  class C4 < AbstractClass
# ^^^^^^^^^^^^^^^^^^^^^^^^ error: Missing definition for abstract method `AbstractClass#bar`
  extend T::Sig
  extend T::Helpers
  sig {override.returns(Object)}
  def self.foo; end
end

  # it handles splats and kwargs
class SplatParent
  extend T::Sig
  extend T::Helpers
  abstract!
  sig { abstract.params(args: Integer, opts: Integer).void }
  def foo(*args, **opts); end
end

class SplatChild1 < SplatParent
  def foo(*args); end # error: Implementation of abstract method `SplatParent#foo` must accept **`opts`
end

class SplatChild2 < SplatParent
  def foo(**opts); end # error: Implementation of abstract method `SplatParent#foo` must accept *`args`
end


module NoSigInInterface
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.returns(Object)}
  def good; end

  def bad; end # error: All methods in an interface must be declared abstract
end


module NonAbstractSigInInterface
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.returns(Object)}
  def good; end

  sig { returns(Object) }
  def bad; end # error: All methods in an interface must be declared abstract
end

module UntypedMixin
  def bad; end
end
module PrivateMethodInInterface
  extend T::Sig
  extend T::Helpers
  interface!
  include UntypedMixin # including untyped mixin in typed interface

  sig {abstract.returns(Object)}
  def good; end
end

module PrivateMethodInInterface
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.returns(Object)}
  private def bad; end # error: Interface method `PrivateMethodInInterface#bad` cannot be private
end

module ProtectedMethodInInterface
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.returns(Object)}
  protected def bad; end # error: Interface method `ProtectedMethodInInterface#bad` cannot be protected
end

module GoodInterface
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.void}
  def foo; end
end

module BadTypedImpl
  extend T::Sig
  extend T::Helpers
  include GoodInterface

  sig {override.returns(Integer)}
  def foo; 1; end # error: Return type `Integer` does not match return type of abstract method `GoodInterface#foo`
end
