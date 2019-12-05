# typed: true
class A
  extend T::Sig

  sig do
    params(
      blk: T.nilable(T.proc.params(r: Integer).returns(Integer))
    )
    .returns(T.untyped)
  end
  def bar(&blk)
  end

  sig do
    params(
      blk: T.proc.params(r: Integer).returns(Integer)
    )
    .returns(T.untyped)
  end
  def baz(&blk)
  end
end

def main
  A.new.bar do |r| # error: Returning value that does not conform to block result type
    r.to_s
  end

  A.new.baz do |r| # error: Returning value that does not conform to block result type
    r.to_s
  end

  A.new.bar do |r|
    r
  end

  A.new.baz do |r|
    r
  end
end
