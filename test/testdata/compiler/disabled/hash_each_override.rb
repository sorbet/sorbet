# frozen_string_literal: true
# typed: true
# compiled: true

# Our intrinsic for Hash#each ought to take into account possible overrides.

extend T::Sig

class Hashlike < Hash
  def initialize
    @called = T.let(false, T::Boolean)
    super
  end

  def each(&blk)
    @called = true
    super
  end
end

h = T.unsafe(Hashlike)["key", "default"]

h.each do |k, v|
  p k
  p v
end

p h.instance_variable_get(:@called)
