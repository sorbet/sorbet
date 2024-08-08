# typed: strict

class Parent; end
class Child < Parent; end

class << self; end

class << Parent; end
#        ^^^^^^ error: `class << EXPRESSION` is only supported for `class << self`
