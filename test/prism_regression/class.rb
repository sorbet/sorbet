# typed: strict

class Parent
  1
  2
  3
end

class Child < Parent; end

class << self
  4
  5
  6
end

class << Parent; end
#        ^^^^^^ error: `class << EXPRESSION` is only supported for `class << self`
