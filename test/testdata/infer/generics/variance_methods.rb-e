# typed: true

module A
  extend T::Sig
  extend T::Helpers
  extend T::Generic

  interface!

  In = type_member(:in)
  Out = type_member(:out)

  # should pass: the parameters are used correctly
  sig {abstract.params(key: In).returns(Out)}
  def lookup(key); end

  # should pass: the parameters are used correctly, and the block argument swaps
  # the polarity of the type members
  sig {abstract.params(key: In, blk: T.proc.params(arg: Out).returns(In)).returns(Out)}
  def lookup_blk(key, &blk); end

  # should fail: the contravariant type key is returned
  sig {abstract.returns(In)}
  def returns_contravariant; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^ error: `type_member` `In` was defined as `:in` but is used in an `:out` context

  # should fail: the contravariant type key is returned
  sig {abstract.returns(T.nilable(In))}
  def returns_nilable_contravariant; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `type_member` `In` was defined as `:in` but is used in an `:out` context

  # should fail: the contravariant type key is given to the block as an argument
  sig {abstract.params(blk: T.proc.params(arg: In).void).void}
                     # ^^^ error: `type_member` `In` was defined as `:in` but is used in an `:out` context
  def block_arg_contravariant(&blk); end

  # should fail: the covariant type key is taken as an argument
  sig {abstract.params(val: Out).void}
                     # ^^^ error: `type_member` `Out` was defined as `:out` but is used in an `:in` context
  def accepts_covariant(val); end

  # should fail: the covariant type key is taken as an argument
  sig {abstract.params(val: T.nilable(Out)).void}
                     # ^^^ error: `type_member` `Out` was defined as `:out` but is used in an `:in` context
  def accepts_nilable_covariant(val); end

  # should fail: the covariant type Out is returned from the block
  sig {abstract.params(blk: T.proc.returns(Out)).void}
                     # ^^^ error: `type_member` `Out` was defined as `:out` but is used in an `:in` context
  def block_returns_covariant(&blk); end
end
