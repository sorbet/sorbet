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
    @foo = T.cast(new, T.attached_class)
  end
end
