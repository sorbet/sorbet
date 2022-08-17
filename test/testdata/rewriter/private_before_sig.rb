# typed: true

class A
  extend T::Sig

  private sig {void}
  def foo; end
end
