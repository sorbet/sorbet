# frozen_string_literal: true
# typed: true
# compiled: true

# This test stresses some weird behaviors with the keyword args singleton where
# because we re-use and re-populate a Ruby Hash object, we can get into a place
# where we need to flip flop between the two different Ruby hash implementations.

class Main
  def self.takes_keyword_args(hash={})
    hash.key?(:prop01)
  end
end

p(Main.takes_keyword_args(
  prop01: 0,
  prop02: 0,
  prop03: 0,
  prop04: 0,
  prop05: 0,
  prop06: 0,
  prop07: 0,
  prop08: 0,
  prop09: 0, # The ninth element causes this hash to overflow the "small hash" implementation.
  prop10: 0, # but we add a bunch more in case Ruby ever decides to define "small" as bigger than 8
  prop11: 0,
  prop12: 0,
  prop13: 0,
  prop14: 0,
  prop15: 0,
  prop16: 0,
  prop17: 0,
  prop18: 0,
  prop19: 0,
))

# This hash should be a small hash, so it should look for prop01 according to the small hash algorithm.
p(Main.takes_keyword_args(
  prop01: 0,
))

