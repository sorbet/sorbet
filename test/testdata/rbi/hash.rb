# typed: strict

Hash.new
Hash.new(0)
Hash.new {|a, e| a[e] = []}

# Bad practise but not a type error
Hash.new([])
