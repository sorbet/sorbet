# typed: true

class Parent
  extend T::Sig
  extend T::Generic

  MyElem = type_member

  sig {overridable.params(x: MyElem).void}
  def example(x)
  end
end
