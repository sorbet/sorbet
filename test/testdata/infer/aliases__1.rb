# typed: strict
module Types
  class T1
  end
end

class Test
  def test
    Types::Alias.new
    T.assert_type!(Types::T1.new, Types::Alias)
    T.assert_type!(Types::Alias.new, Types::T1)
  end
end
