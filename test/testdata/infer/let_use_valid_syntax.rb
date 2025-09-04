# typed: strict

class A
  extend T::Sig

  sig {returns(T::Array[T.attached_class])}
  def self.instances
    @instances ||= T.let([new, new], T.nilable(T::Array[T.attached_class]))
  end
end

class B
  extend T::Sig
  sig { void }
  def self.foo
    @foo = T.cast(new, T.nilable(T.attached_class))
    #      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Use `T.let` to specify the type of instance variables
    #      ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `T.cast` is useless because `T.attached_class (of B)` is already a subtype of `T.nilable(T.attached_class (of B))`
  end
end
