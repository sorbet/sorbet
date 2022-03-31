# typed: true

class DoesntHaveSig
  sig do
# ^^^ error: Method `sig` does not exist
    void
  # ^^^^ error: Method `void` does not exist
  end
  def foo; end
end

class HasSig
  extend T::Sig

  sig {void}
  def foo; end
end

  sig do
# ^^^ error: Method `sig` does not exist
    void
  # ^^^^ error: Method `void` does not exist
  end
  def foo; end
