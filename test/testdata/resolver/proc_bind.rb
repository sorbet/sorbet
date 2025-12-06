# typed: true

class TestProcBind
  extend T::Sig

  sig { params(blk: T.nilable(T.proc.bind(String).void)).void }
  def nilable_bind(&blk)
    0
  end

  sig { params(blk: T.proc.bind(String).void).void }
  def bind(&blk)
    0
  end

  sig { params(blk: T.nilable(T.proc.void)).void }
  def nilable(&blk)
    0
  end

  sig { params(blk: T.nilable(T.nilable(T.proc.void))).void }
  def double_nilable(&blk)
    0
  end

  sig { params(blk: T.nilable(T.nilable(T.proc.bind(String).void))).void }
  def double_nilable_bind(&blk)
    0
  end
end

TestProcBind.new.nilable_bind

TestProcBind.new.nilable_bind do
  T.reveal_type(self) # error: Revealed type: `String`
end

TestProcBind.new.bind do
  T.reveal_type(self) # error: Revealed type: `String`
end

TestProcBind.new.nilable do
  T.reveal_type(self) # error: Revealed type: `T.self_type (of T.class_of(<root>))`
end

TestProcBind.new.double_nilable do
  T.reveal_type(self) # error: Revealed type: `T.self_type (of T.class_of(<root>))`
end

TestProcBind.new.double_nilable_bind do
  T.reveal_type(self) # error: Revealed type: `String`
end
