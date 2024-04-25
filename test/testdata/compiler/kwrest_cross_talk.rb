# frozen_string_literal: true
# typed: true
# compiled: true

X = {key: 'good value'}

def foo(**kwargs)
  comparison = X.equal?(kwargs)
  kwargs[:key] = 'bad value'
  comparison
end

puts foo(X)
puts
p X
