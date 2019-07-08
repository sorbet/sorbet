# typed: true

class A
  extend T::Sig
  extend T::Helpers
  extend T::Generic

  abstract!

  Key = type_member(:in) # error: Classes can only have invariant type members
  Val = type_member(:out)

  # should pass: the parameters are used correctly
  sig {abstract.params(key: Key).returns(Val)}
  def lookup(key); end

  # should pass: the parameters are used correctly, and the block argument swaps
  # the polarity of the type members
  sig {abstract.params(key: Key, blk: T.proc.params(arg: Val).returns(Key)).returns(Val)}
  def lookup(key, &blk); end

  # should fail: the contravariant type key is returned
  sig {abstract.returns(Key)}
  def returns_contravariant; end

  # should fail: the contravariant type key is returned
  sig {abstract.returns(T.nilable(Key))}
  def returns_nilable_contravariant; end

  # should fail: the contravariant type key is given to the block as an argument
  sig {abstract.params(blk: T.proc.params(arg: Key).void).void}
  def block_arg_contravariant(&blk); end

  # should fail: the covariant type key is taken as an argument
  sig {abstract.params(val: Val).void}
  def accepts_covariant(val); end

  # should fail: the covariant type key is taken as an argument
  sig {abstract.params(val: T.nilable(Val)).void}
  def accepts_nilable_covariant(val); end

  # should fail: the covariant type Val is returned from the block
  sig {abstract.params(blk: T.proc.returns(Val)).void}
  def block_returns_covariant(&blk); end
end
