# typed: true

class A
  extend T::Sig

  private sig {void}
  #       ^^^^^^^^^^ error: Expected `Symbol` but found `NilClass` for argument `method_name`
  def foo; end
end
