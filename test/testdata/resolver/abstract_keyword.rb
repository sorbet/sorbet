# typed: true
# enable-experimental-method-modifiers: true

module ShimForAbstractModifierSupport
  # We can't use `T::DefMods` directly (since it's not in Sorbet's payload), so we provide a no-op shim just like it.
  def abstract(method_name) = method_name
end

class Module
end

module AbstractKeywordInInterface
  extend T::Helpers
  extend ShimForAbstractModifierSupport

  interface!

  abstract def foo; end
  abstract def bar(x, y); end

  abstract def self.class_method; end

  public abstract def pub1; end
  protected abstract def prot1; end # error: cannot be protected
  private abstract def priv1; end

  abstract public def pub2; end
  abstract protected def prot2; end # error: cannot be protected
  abstract private def priv2; end
end

class AbstractKeywordInClass
  extend T::Helpers
  extend ShimForAbstractModifierSupport

  abstract!

  abstract def self.class_method; end

  public abstract def pub1; end
  protected abstract def prot1; end
  private abstract def priv1; end

  abstract public def pub2; end
  abstract protected def prot2; end
  abstract private def priv2; end
end

module KeywordAndSig
  class Parent
    extend T::Sig
    extend T::Helpers
    extend ShimForAbstractModifierSupport

    abstract!

    sig { returns(Integer) }
    abstract def foo; end
  end

  class Child < Parent
    abstract!

    sig { override.returns(Integer) }
    def foo = 123
  end
end

module KeywordAndSigRundantAbstract
  class Parent
    extend T::Sig
    extend T::Helpers
    extend ShimForAbstractModifierSupport

    abstract!

    sig { abstract.returns(Integer) }
    abstract def foo; end
  end

  class Child < Parent
    abstract!

    sig { override.returns(Integer) }
    def foo = 123
  end
end
