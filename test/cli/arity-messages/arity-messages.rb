# typed: true

def foo(x: nil); end
foo 1
foo(y: 1)

def bar(x: nil, y: nil); end
bar(5, x: 8)
bar(5, 8)

def baz(x: nil); end
baz 7

def quux(x, y, z); end
quux(2, z: 3)

def qaar(a, b, c: nil, d: nil); end
qaar(1, 2, 3, c: nil, d: nil)

def qaadr(e: 1); end
qaadr({a: 1}, {b: 2}, {c: 3})

def pippo(x, **kwargs); end
pippo(2, 3, z: 4)

def pluto(x, *args); end
pluto

def paperino(x, y: nil, **kwargs); end
paperino(2, 3, z: 4)

class A
  def self.make(foo:, bar:, qux:)
  end
end

A.make(
  foo: [1, 2, 3],
  bar: "some long string",
)

def requires_multiple_keyword_args(x:, y:)
end

requires_multiple_keyword_args
