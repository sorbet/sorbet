# typed: strict

foo = T.let(nil, T.nilable(String))
foo[0]

"hi" + foo

sig(a: String).void
def int(a:); end
int(a: foo)

foo.bar ||= "a"
foo.bar &&= "a"
foo.bar |= "a"
foo.bar &= "a"

[1].max_by {|l,r| 1}[2]

sig(a: T.nilable(Integer)).void
def foo(a:)
end
a = T.let(nil, T.nilable(Numeric))
foo(a: a)
