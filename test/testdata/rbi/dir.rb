# typed: strict

extend T::Sig

sig {returns(T::Array[String])}
def test_dir_brackets
  Dir['a', 'b']
  Dir['a', base: '.']
  Dir['/*'] do |match|
    T.assert_type!(match, String)
  end
  Dir[Pathname.new('.')]
end
