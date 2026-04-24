# typed: true
# enable-experimental-method-modifiers: true

module T::Helpers
  # Not implemented in sorbet-runtime yet, so lets add our own no-op shim here for now.
  def abstract(method_name) = method_name
end

module AbstractKeywordInInterface
  extend T::Helpers
  interface!

  abstract def foo; end
  abstract def bar(x, y); end

  public abstract def pub1; end
  protected abstract def prot1; end # error: cannot be protected
  private abstract def priv1; end
  
  abstract public def pub2; end
  abstract protected def prot2; end # error: cannot be protected
  abstract private def priv2; end
end

class AbstractKeywordInClass
  extend T::Helpers
  abstract!

  public abstract def pub1; end
  protected abstract def prot1; end
  private abstract def priv1; end

  abstract public def pub2; end
  abstract protected def prot2; end
  abstract private def priv2; end
end
