# frozen_string_literal: true
# typed: true
# compiled: true

# Our intrinsic for Hash#each_with_block ought to take into account possible overrides.

extend T::Sig

class Hashlike < Hash
  def initialize
    @called = T.let(false, T::Boolean)
    super
  end

  def each_with_block(obj, &blk)
    @called = true
    super
  end
end

h = T.unsafe(Hashlike)["key", "default"]

result = h.each_with_block([]) do |kv, array|
  array << kv
end

p result
p h.instance_variable_get(:@called)
