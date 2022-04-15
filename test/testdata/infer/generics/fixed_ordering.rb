# typed: true

class C
  extend T::Sig

  sig {params(f: Fixed).returns(NilClass)}
  def test_it(f)
    T.assert_type!(f.first, T.nilable(String))
    nil
  end
end

class Fixed
  include Enumerable
  extend T::Generic

  Elem = type_member {{fixed: String}}

  def each(&blk); end
end
