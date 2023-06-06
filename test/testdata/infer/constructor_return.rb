# typed: true
class A
  extend T::Sig

  sig {params(blk: T.proc.void).void}
  def initialize(&blk)
    yield
    1
  end
end

b = A.new {}
T.assert_type!(b, A)
