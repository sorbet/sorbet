# typed: strict

class Parent
  extend T::Sig
  extend T::Generic

  MyElem = type_template
end

  class Child < Parent
# ^^^^^^^^^^^^^^^^^^^^ error: Type `MyElem` declared by parent `T.class_of(Parent)` must be re-declared in `T.class_of(Child)`
  end
