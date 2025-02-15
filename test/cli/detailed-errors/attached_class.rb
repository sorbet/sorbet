# typed: true

class AttachedClassA
  extend T::Sig

  sig {returns(T.attached_class)}
  def self.make
    AttachedClassA.new
  end
end

class AttachedClassB
  extend T::Sig

  sig {returns(T::Array[T.attached_class])}
  def self.make
    [AttachedClassB.new]
  end
end

