# typed: true
class A
  extend T::Helpers

  sig {params(s: T.nilable(String)).returns(NilClass)}
  def test_return(s)
    if s.nil?
      return
    end

    s.length
    nil
  end
end
