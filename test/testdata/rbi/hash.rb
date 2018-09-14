# typed: true

Hash.new
Hash.new(0)
h = Hash.new {|a, e| a[e] = []}
h[:foo] = :bar

# Bad practise but not a type error
Hash.new([])

# to_a and Hash[] play nice together
T.assert_type!(
  Hash[T::Hash[String, String].new.to_a],
  T::Hash[String, String]
)
