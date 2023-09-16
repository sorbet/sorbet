# typed: true

TypeAlias = T.type_alias { Integer }

class A
  extend T::Sig
  extend T::Generic

  X = type_member

  Const = 0

  sig {returns(T.class_of(X))}
  #            ^^^^^^^^^^^^^ error: T.class_of can't be used with a T.type_member
  def example1; end

  sig {returns(T.class_of(TypeAlias))}
  #            ^^^^^^^^^^^^^^^^^^^^^ error: T.class_of can't be used with a T.type_alias
  def example2; end

  sig {returns(T.class_of(Const))}
  #            ^^^^^^^^^^^^^^^^^ error: T.class_of can't be used with a constant field
  #                       ^^^^^ error: Unexpected bare `Integer` value found in type position
  def example3; end

  sig {returns(T.class_of(T.any(Integer, String)))}
  #            ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `T.class_of` must wrap each individual class type, not the outer `T.any`
  def example4; end

  sig {returns(T.class_of(T.any(X, String)))}
  #            ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `T.class_of` needs a class or module as its argument
  def example5; end
end
