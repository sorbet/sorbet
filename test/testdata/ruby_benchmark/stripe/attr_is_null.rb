# frozen_string_literal: true
# typed: true
# compiled: true

# ::Boolean copied from extn/boolean.rb

module ::Boolean
  extend T::Helpers
  sealed!
end

class ::FalseClass
  include ::Boolean
end

class ::TrueClass
  include ::Boolean
end

# End code copied from extn/boolean.rb

# Thing#has_some_attr? is analogous to Opus::APIErrors::Code#documented?
class Thing
  extend T::Sig

  sig {returns(T.nilable(String))}
  attr_reader :some_attr

  sig {params(some_attr: T.nilable(String)).void}
  def initialize(some_attr:)
    @some_attr = some_attr
  end

  # Note that as of this writing, if we replace Boolean with T::Boolean in the signature, interpreted is a little more
  # than twice as slow, while compiled runs at about the same speed.
  sig {returns(Boolean)}
  def has_some_attr?
    !@some_attr.nil?
  end
end

e = Thing.new(some_attr: "hello")

i = 0
while i < 10_000_000
  e.has_some_attr?

  i += 1
end

puts i
