# frozen_string_literal: true
# typed: true
# compiled: true

def bar(x)
  y = T.let(x, T.nilable(String))
  y.to_s
end

p bar('baz')
p bar(nil)

