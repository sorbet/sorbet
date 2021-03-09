# typed: true

foo = T.let(nil, T.nilable(String))
foo[0]

"hi" + foo

T::Array[T.nilable(Integer)].new.map(&:even?)
