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
  A.new.bar do |r|
    r.to_s
  # ^^^^^^ error: Expected `Integer` but found `String` for block result type
  end

  A.new.baz do |r|
    r.to_s
  # ^^^^^^ error: Expected `Integer` but found `String` for block result type
  end

  A.new.bar do |r|
    r
  end

  A.new.baz do |r|
    r
  end
end
