# typed: true

class A
  extend T::Sig

  private sig {void}
  #       ^^^^^^^^^^ error: Expected `T.all(T.type_parameter(:U), T.any(Symbol, String))` but found `NilClass` for argument `method_name`
  def foo; end
end
