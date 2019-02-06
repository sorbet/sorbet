# typed: true

tf = Tempfile.new
T.reveal_type(tf) # error: Revealed type: `Tempfile`
T.let(T.must(tf.path), String)
T.let(tf.length, Integer)
T.let(tf.size, Integer)
T.let(tf.unlink, T.any(TrueClass, FalseClass))

a_str_or_tf = Tempfile.open('example_basename', 'example_tempdir', 'r', {foo: 'bar'}) do |tf|
  T.let(tf, Tempfile)
  "a string"
end
if a_str_or_tf.is_a?(Tempfile)
  T.let(a_str_or_tf, Tempfile)
  # Test out some File methods
  a_str_or_tf.seek(10)
  a_str_or_tf.rewind
  a_str_or_tf.path
elsif a_str_or_tf.is_a?(String)
  T.let(a_str_or_tf, T.nilable(String))
  a_str_or_tf
end

an_int_or_file = Tempfile.create('base', 'dir', 'w', {foo: 'bar'}) do |f|
  T.let(f, File)
  12345
end
if an_int_or_file.is_a?(File)
  T.let(an_int_or_file, File)
  an_int_or_file.seek(100)
  an_int_or_file.write('foo bar baz')
elsif an_int_or_file.is_a?(Integer)
  T.let(an_int_or_file, Integer) + 100
end
