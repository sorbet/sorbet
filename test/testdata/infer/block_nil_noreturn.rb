# typed: true
extend T::Sig

sig { params(blk: T.noreturn).returns(Integer) }
def takes_no_block(&blk)
  T.reveal_type(blk) # error: This code is unreachable
  0
end

sig { params(blk: T.noreturn).returns(Integer) }
def takes_no_block_does_not_use_it(&blk)
  T.reveal_type(0) # error: `Integer(0)`
end

f = ->(){}

p(takes_no_block())
#                 ^ error: requires a block parameter, but no block was passed
p(takes_no_block() {})
p(takes_no_block(&nil))
# ^^^^^^^^^^^^^^^^^^^^ error: Expected `T.noreturn` but found `NilClass` for block argument
p(takes_no_block(&f))
# ^^^^^^^^^^^^^^^^^^ error: Expected `T.noreturn` but found `T.proc.returns(NilClass)` for block argument

sig { params(blk: NilClass).returns(Integer) }
def takes_nil_block(&blk)
  T.reveal_type(blk) # error: `NilClass`
  0
end

p(takes_nil_block())
p(takes_nil_block() {})
p(takes_nil_block(&nil))
p(takes_nil_block(&f))
# ^^^^^^^^^^^^^^^^^^^ error: Expected `NilClass` but found `T.proc.returns(NilClass)`

