# typed: true

class A
  extend T::Sig

  private sig {void}
  #       ^^^^^^^^^^ error: Expected `T.any(Symbol, String)` but found `NilClass` for argument `arg0`
  def foo; end
end
