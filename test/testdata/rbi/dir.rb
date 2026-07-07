# typed: strict

extend T::Sig

sig {returns(T::Array[String])}
def test_dir_brackets
  Dir['a', 'b']
  Dir['a', base: '.']
  Dir['/*']
  Dir[Pathname.new('.')]
end

sig {void}
def test_dir_brackets_no_block
  # Dir.[] does not yield (unlike Dir.glob); a block is silently ignored at
  # runtime, so passing one is a latent bug. Sorbet flags it.
  Dir['*'] { |f| f } # error: does not take a block
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
  Dir.glob('*.rb', base: 'lib')
  Dir.glob('*.rb', base: Pathname.new('lib'))
  Dir.glob('*.rb', sort: false)
  Dir.glob('*.rb', File::FNM_DOTMATCH, base: 'lib', sort: true)
  Dir.glob('*.rb')
end

sig {params(enc: T.nilable(Encoding)).void}
def test_dir_children(enc)
  T.assert_type!(Dir.children('.'), T::Array[String])
  T.assert_type!(Dir.children(Pathname.new('.')), T::Array[String])
  Dir.children('.', encoding: Encoding::UTF_8)
  Dir.children('.', encoding: 'UTF-8')
  Dir.children('.', encoding: nil)
  Dir.children('.', encoding: enc)
end

sig {void}
def test_dir_each_child
  Dir.each_child(Pathname.new('.')) do |child|
    T.assert_type!(child, String)
  end
  Dir.each_child('.', encoding: Encoding::UTF_8) {|child| child}
  Dir.each_child('.', encoding: 'UTF-8') {|child| child}
  Dir.each_child('.', encoding: nil) {|child| child}
  T.assert_type!(Dir.each_child('.'), T::Enumerator[String])
end

sig {void}
def test_dir_entries
  T.assert_type!(Dir.entries('.'), T::Array[String])
  T.assert_type!(Dir.entries(Pathname.new('.')), T::Array[String])
  Dir.entries('.', encoding: Encoding::UTF_8)
  Dir.entries('.', encoding: 'UTF-8')
  Dir.entries('.', encoding: nil)
end

sig {void}
def test_dir_foreach
  Dir.foreach(Pathname.new('.')) do |entry|
    T.assert_type!(entry, String)
  end
  Dir.foreach('.', encoding: Encoding::UTF_8) {|entry| entry}
  Dir.foreach('.', encoding: 'UTF-8') {|entry| entry}
  Dir.foreach('.', encoding: nil) {|entry| entry}
  T.assert_type!(Dir.foreach('.'), T::Enumerator[String])
end

sig {void}
def test_dir_open
  T.assert_type!(Dir.open('.'), Dir)
  T.assert_type!(Dir.open(Pathname.new('.'), encoding: Encoding::UTF_8), Dir)
  T.assert_type!(Dir.open('.', encoding: 'UTF-8'), Dir)
  T.assert_type!(Dir.open('.', encoding: nil), Dir)
  result = Dir.open('.') do |dir|
    T.assert_type!(dir, Dir)
    42
  end
  T.assert_type!(result, Integer)
end

sig {void}
def test_dir_new
  T.assert_type!(Dir.new('.'), Dir)
  T.assert_type!(Dir.new(Pathname.new('.')), Dir)
  T.assert_type!(Dir.new('.', encoding: Encoding::UTF_8), Dir)
  T.assert_type!(Dir.new('.', encoding: 'UTF-8'), Dir)
  T.assert_type!(Dir.new('.', encoding: nil), Dir)
end

sig {params(dir: Dir).void}
def test_dir_instance_children(dir)
  T.assert_type!(dir.children, T::Array[String])
end

sig {params(dir: Dir).void}
def test_dir_instance_each_child(dir)
  dir.each_child do |child|
    T.assert_type!(child, String)
  end
  T.assert_type!(dir.each_child {|child| child}, Dir)
  T.assert_type!(dir.each_child, T::Enumerator[String])
end

sig {void}
def test_dir_empty_and_delete
  T.assert_type!(Dir.empty?('.'), T::Boolean)
  T.assert_type!(Dir.empty?(Pathname.new('.')), T::Boolean)
  T.assert_type!(Dir.delete('x'), Integer)
  T.assert_type!(Dir.delete(Pathname.new('x')), Integer)
  T.assert_type!(Dir.chroot(Pathname.new('/')), Integer)
end
