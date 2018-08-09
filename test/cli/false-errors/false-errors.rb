# typed: strict
class K
end

module B
  def foo
  end
end

T.cast(T.unsafe(nil), T.all(K, B)).foo