# typed: true

class Parent
  extend T::Sig, T::Generic
  abstract!

  MType = type_member

  sig { overridable.type_parameters(:U).params(foo: T.all(MType, Integer)).returns(T.type_parameter(:U)) }
  def example(foo)
    raise
  end
end

class Child < Parent
  MType = type_member

  sig { override.type_parameters(:U).params(foo: T.all(MType, Integer)).returns(T.type_parameter(:U)) }
  def example(foo)
    raise
  end
end
