# typed: strict
class Test
  sig(b: T.nilable(Integer)).returns(T.untyped)
  def bad(b)
    T.unsafe(nil) && b && b < 0
  end
end
