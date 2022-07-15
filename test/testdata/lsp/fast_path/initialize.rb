# typed: true
# no-stdlib: true
# ^ this is a hack so that we can test whether this takes the fast/slow path
#   without having to hard code the list of every payload file that mentions
#   `initialize`

module Sorbet::Private::Static
  def self.keep_for_ide(expr)
  end
end

module T
  def self.reveal_type(expr)
  end
end

class A
  def self.extend(*mod)
  end

  extend T::Sig
  sig {params(x: Integer).void}
  #    ^^^^^^ error: `params` does not exist
  def initialize(x)
    T.reveal_type(x) # error: `Integer`
  end
end
