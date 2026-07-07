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

sig {returns(T::Array[String])}
def test_dir_glob
  Dir.glob('*.rb')
  Dir.glob(['*.rb', '*.h'])
  Dir.glob(Pathname.new('.'))
  Dir.glob([Pathname.new('a'), 'b'])
  Dir.glob(Pathname.new('.')) do |match|
    T.assert_type!(match, String)
  end
  Dir.glob('*.rb')
end
