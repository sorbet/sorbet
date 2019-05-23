# typed: true
extend T::Sig
sig do
  params(
    a: Integer,
    b: Integer,
    c: Integer,
    d: Integer,
    e: Integer,
    f: Integer,
  )
    .void
end
def foo(a:, b:, c:, d:, e:, f:); end

foo(
  a: 1,
  b: 1,
  c: 1,
  d: "1",
  e: 1,
  f: 1,
)

sig do
  params(
    a: Integer,
    b: Integer,
    c: Integer,
    d: Integer,
    e: Integer,
    f: Integer,
  )
    .void
end
def bar(a, b, c, d, e, f); end

bar(
  1,
  1,
  1,
  "1",
  1,
  1,
)
