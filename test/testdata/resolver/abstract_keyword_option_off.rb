# typed: true
# (enable-experimental-method-modifiers is not set, so it should default to false)

module ShimForAbstractModifierSupport
  # We can't use `T::DefMods` directly (since it's not in Sorbet's payload), so we provide a no-op shim just like it.
  def abstract(method_name) = method_name
end

class AbstractKeywordInClass
  extend T::Helpers
  extend ShimForAbstractModifierSupport

  abstract!

  abstract def foo; end
  abstract def bar(x, y); end

  public abstract def pub1; end
  protected abstract def prot1; end
  private abstract def priv1; end

  abstract public def pub2; end
  abstract protected def prot2; end
  abstract private def priv2; end
end
