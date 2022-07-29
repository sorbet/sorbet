# typed: strict

extend T::Sig

sig {void}
def test_find_find
  T.assert_type!(Find.find('.'), T::Enumerator[String])
  Find.find('.') do |match|
    T.assert_type!(match, String)
  end
end
