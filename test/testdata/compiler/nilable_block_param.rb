# frozen_string_literal: true
# typed: strict
# compiled: true

extend T::Sig
sig {params(blk: T.nilable(T.proc.void)).void}
def foo(&blk)
  yield if block_given?
  140
end

puts(foo)
#    ^^^ error: Expected `Object` but found `Sorbet::Private::Static::Void` for argument `arg0`
puts(foo {puts 1})
#    ^^^^^^^^^^^^ error: Expected `Object` but found `Sorbet::Private::Static::Void` for argument `arg0`
