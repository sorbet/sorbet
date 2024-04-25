# typed: true
# frozen_string_literal: true
# compiled: false

require_relative './hash_literal_gc__2'

# Change the following 42 to a 0 and it will _probably_ work.
42.times do
  GC.start
  T.unsafe(GC).verify_compaction_references(double_heap: true, toward: :empty)
end

p HasAHash::give_me_the_hash
