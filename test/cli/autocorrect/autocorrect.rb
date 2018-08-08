# typed: strict

foo = T.let(nil, T.nilable(String))
foo[0]

"hi" + foo

foo.bar ||= "a"
foo.bar &&= "a"
