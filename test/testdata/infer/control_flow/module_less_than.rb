# typed: true
extend T::Sig

class A
  def self.foo(x)
    x.to_s
  end
end

sig {params(y: Object).void}
def main(y)
  if y.is_a?(Module) && y < A
    y.foo("bar")
  end
end

module M; end

sig { params(obj: T.untyped).void }
def valid?(obj)
  T.reveal_type(obj) # error: `T.untyped`
  if obj.is_a?(Class)
    T.reveal_type(obj) # error: `T::Class[T.anything]`

    if obj <= T.unsafe(Integer)
      T.reveal_type(obj) # error: `T::Class[T.anything]`
    end

    if obj <= Integer
      T.reveal_type(obj) # error: `T.class_of(Integer)`
    end

    if obj <= M
      T.reveal_type(obj) # error: `T::Class[M]`
    end
  end

  T.reveal_type(obj) # error: `T.untyped`
  if obj.is_a?(Module)
    T.reveal_type(obj) # error: `T::Module[T.anything]`

    if obj <= T.unsafe(Integer)
      T.reveal_type(obj) # error: `T::Module[T.anything]`
    end

    if obj <= Integer
      T.reveal_type(obj) # error: `T.class_of(Integer)`
    end

    if obj <= M
      T.reveal_type(obj) # error: `T::Module[T.anything]`
    end
  end
end

module Exportable
  extend T::Sig
  sig { returns(Integer) }
  def export
    0
  end
end

sig { params(klass: T.class_of(T::Enum)).void }
def class_lt_module(klass)
  if klass < Exportable
    T.reveal_type(klass) # error: `T.class_of(T::Enum)[T.all(T::Enum, Exportable)]`
    value = klass.try_deserialize("foo")
    raise unless value
    exported = value.export
    T.reveal_type(exported) # error: `Integer`
  end
end

module FromHash
  extend T::Sig, T::Generic
  has_attached_class!
  abstract!

  sig { abstract.returns(T.attached_class) }
  def from_hash
  end
end

sig { params(klass: T.all(Module, FromHash[T.anything])).void }
def module_lt_module(klass)
  if klass < Exportable
    T.reveal_type(klass) # error: `T.all(T::Module[T.anything], FromHash[T.anything])`
    value = klass.from_hash
    T.reveal_type(value) # error: `T.anything`
    raise unless value
    exported = value.export # error: Method `export` does not exist on `T.anything`
    T.reveal_type(exported) # error: `T.untyped`
  end
end

module HasClassMethods
  extend T::Helpers

  module ClassMethods
  end
  mixes_in_class_methods(ClassMethods)
end

sig { params(klass: T::Class[T.anything], mod: Module).void }
def nothing_special_for_class_methods(klass, mod)
  if klass < HasClassMethods
    T.reveal_type(klass) # error: `T::Class[HasClassMethods]`
  end

  if mod < HasClassMethods
    T.reveal_type(mod) # error: `T::Module[T.anything]`
  end
end

sig { params(other: T.class_of(A)).void }
def module_should_not_add_noTypeTest(other)
  if other <= M
    T.reveal_type(other) # error: `T.class_of(A)[T.all(A, M)]`
    return
  end

  T.reveal_type(other) # error: `T.class_of(A)`
end
