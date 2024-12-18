# typed: true

class AbstractClassVisibility
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.returns(Object)}
  def foo; end

  sig {abstract.returns(Object)}
  def bar; end

  sig {abstract.returns(Object)}
  def self.class_foo; end
end

class ImplVisibility < AbstractClassVisibility
  sig {override.returns(Object)}
  protected def foo; end
          # ^^^^^^^ error: Method `foo` is protected in `ImplVisibility` but not in `AbstractClassVisibility`
  private def bar; end
        # ^^^^^^^ error: Method `bar` is private in `ImplVisibility` but not in `AbstractClassVisibility`

  sig {override.returns(Object)}
  def self.class_foo; end
  # error: Method `class_foo` is private in `T.class_of(ImplVisibility)` but not in `T.class_of(AbstractClassVisibility)`
  private_class_method :class_foo
end
