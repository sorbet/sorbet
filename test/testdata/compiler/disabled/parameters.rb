# typed: true

def foo(a, b=1, c:, d:2, **e)
end

def bar(*args)
end

def baz(*)
end

puts method(:foo).parameters
puts method(:bar).parameters
puts method(:baz).parameters
