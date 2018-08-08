# typed: strict

foo = T.let(nil, T.nilable(String))
foo[0]

"hi" + foo

sig(a: String).void
def int(a:); end
int(a: foo)

foo.bar ||= "a"
foo.bar &&= "a"
