# typed: true

class C
  extend T::Helpers

  sig {params(f: Fixed).returns(NilClass)}
  def test_it(f)
    # TODO(RUBYPLAT-520): This should pass, but because Fixed is
    # defined lower in the file, we don't yet see the resultType of
    # the fixed member.
    T.assert_type!(f.first, T.nilable(String)) # error: The typechecker was unable to infer the type of the asserted value
    nil
  end
end

class Fixed
  include Enumerable
  extend T::Generic

  Elem = type_member(fixed: String)

  def each; end
end
