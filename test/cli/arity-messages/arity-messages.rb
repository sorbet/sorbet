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

def pippo(x, **kwargs); args; end
pippo(2, 3, z: 4)

def paperino(x, y: nil, **kwargs); args; end
paperino(2, 3, z: 4)
