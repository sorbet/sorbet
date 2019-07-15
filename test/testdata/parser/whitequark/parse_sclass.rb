# typed: true

class << foo; nil; end # error-with-dupes: `class << EXPRESSION` is only supported for `class << self`
