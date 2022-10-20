# typed: true
extend T::Sig

class A
  extend T::Sig

  sig {returns(A)}
  def to_proc
    # The Ruby VM raises an exception when the result of calling `to_proc` for
    # a blockpass does not produce a `Proc` object.
    self
  end

  sig {void}
  def call
  end
end

sig {params(blk: A).void}
#           ^^^ error: Block argument type must be either `Proc` or a `T.proc` type (and possibly nilable)
def takes_a_as_block(&blk)
end

[].each(&A.new) # error: Expected `T.proc.params(arg0: T.untyped).returns(BasicObject)` but found `A` for block argument

# The above error about block argument type sets the block type to `T.untyped`,
# which means neither of these are errors.
takes_a_as_block(&A.new)
takes_a_as_block(&(->(){}))
