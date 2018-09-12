# typed: true

Hash.new
Hash.new(0)
h = Hash.new {|a, e| a[e] = []}
h[:foo] = :bar

# Bad practise but not a type error
Hash.new([])
