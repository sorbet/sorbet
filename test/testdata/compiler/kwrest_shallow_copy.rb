# frozen_string_literal: true
# typed: true
# compiled: true

X = {key: []}

def foo(**kwargs)
  kwargs[:key] << 'hello there'
end

foo(X)
p X
