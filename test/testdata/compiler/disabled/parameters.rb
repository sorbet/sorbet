# typed: true

def foo(a, b=1, c:)
end

puts method(:foo).parameters
