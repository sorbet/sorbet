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

  sig {returns(T::Boolean)}
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
