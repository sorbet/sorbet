# frozen_string_literal: true
# typed: strict
# compiled: true

extend T::Sig
sig {params(blk: T.nilable(T.proc.void)).returns(Integer)}
def foo(&blk)
  yield if block_given?
  140
end

puts(foo)
puts(foo {puts 1})
