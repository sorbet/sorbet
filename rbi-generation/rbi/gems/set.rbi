# typed: true

# require 'set' monkey patches stuff onto Array that we don't have in our core
# gem shims.

class Array
  sig {returns(Set)}
  def to_set; end
end
