# typed: true

class A
  extend T::Sig

  sig {params(blk: T.proc.bind(A).void).void}
  def self.thing(&blk)
  end

  sig {params(blk: T.proc.bind(T.attached_class).void).void}
  def self.bind_attached(&blk)
  end

  sig {params(blk: T.proc.bind(T.self_type).void).void}
  def self.bind_self(&blk)
  end

  sig {params(blk: T.proc.void).void}
  def self.maybe_bind(&blk)
  end

end

class B
  extend T::Sig

  sig {params(blk: T.proc.bind(B).void).void}
  def self.thing(&blk)
  end

  sig {params(blk: T.proc.bind(T.attached_class).void).void}
  def self.bind_attached(&blk)
  end

  sig {params(blk: T.proc.bind(T.self_type).void).void}
  def self.bind_self(&blk)
  end

  sig {params(blk: T.proc.bind(B).void).void}
  def self.maybe_bind(&blk)
  end

end

x = T.let(A, T.any(T.class_of(A), T.class_of(B)))

T.reveal_type(x) # error: `T.any(T.class_of(A), T.class_of(B))`

x.thing do
  T.reveal_type(self) # error: `T.any(A, B)`
end

x.bind_attached do
  T.reveal_type(self) # error: `T.any(A, B)`
end

x.bind_self do
  T.reveal_type(self) # error: `T.any(T.class_of(A), T.class_of(B))`
end

x.maybe_bind do
  T.reveal_type(self) # error: `T.any(B, T.self_type (of T.class_of(<root>)))`
end
