# frozen_string_literal: true
# typed: true
# compiled: true

extend T::Sig

sig {params(x: Float, y: T.untyped).returns(T.untyped)}
def plus(x, y)
  x + y
end

sig {params(x: Float, y: T.untyped).returns(T.untyped)}
def minus(x, y)
  x - y
end

sig {params(x: Float, y: T.untyped).returns(T::Boolean)}
def lt(x, y)
  x < y
end

sig {params(x: Float, y: T.untyped).returns(T::Boolean)}
def lte(x, y)
  x <= y
end

p plus(5.0, 6.0)
p minus(34.3, 89.4)
p lt(87.2, 5.9)
p lt(6.8, 27.5)
p lte(64.1, 41.9)
p lte(6.4, 41.9)
p lte(8.8, 8.8)

# Test some non-float 2nd args to make sure our called intrinsics work properly.
p plus(5.0, 8)
p plus(4.0, 8.9r)
p plus(4.0, 5i)

p minus(98.7, 3)
p minus(98.6, 15.4r)
p minus(98.5, 18i)

p lt(18.2, 6)
p lt(18.2, 25.4r)

p lte(18.2, 20)
p lte(18.2, 5.923r)
