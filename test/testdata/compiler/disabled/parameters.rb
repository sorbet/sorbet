# typed: true

def foo(a, b=1, *c, d:, e:2, **f)
end

def bar(*)
end

puts method(:foo).parameters
puts method(:bar).parameters
