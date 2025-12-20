# frozen_string_literal: true
# typed: true
# compiled: true

# Hash#each_with_object is actually implemented via #each, so we should either
# use the overridden version in our intrinsic or we should disable use of our
# intrinsic entirely.  Both options would make this test pass.

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

result = h.each_with_block([]) do |kv, array|
  array << kv
end

p result
p h.instance_variable_get(:@called)
