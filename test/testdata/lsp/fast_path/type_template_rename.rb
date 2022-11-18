# typed: strict
# disable-fast-path: true
class Parent
  extend T::Sig
  extend T::Generic

  MyElem = type_template

  sig {params(x: MyElem).void}
  def self.example1(x)
  end

  sig {params(x: AnotherElem).void}
  #              ^^^^^^^^^^^ error: Unable to resolve constant `AnotherElem`
  def self.example2(x)
  end
end

  class Child < Parent
# ^^^^^^^^^^^^^^^^^^^^ error: Type `MyElem` declared by parent `T.class_of(Parent)` must be re-declared in `T.class_of(Child)`
  end
