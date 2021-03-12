# typed: strict

foo = T::Array[Integer].new
foo.inject(nil) {|sum, x| sum ? sum + x : x}
