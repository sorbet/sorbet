# typed: true

def foo(x: nil); end
foo 1

def bar(x: nil, y: nil); end
bar(5, x: 8)

def baz(x: nil); end
baz 7
