# typed: __STDLIB_INTERNAL

class Object < BasicObject
  include Kernel

  sig {returns(Integer)}
  def object_id(); end

  sig {params(blk: T.proc.params(arg: T.untyped).returns(T.untyped)).returns(T.untyped)}
  def yield_self(&blk); end

  # `then` is just an alias of `yield_self`. Separately def'd here for easier IDE integration
  sig {params(blk: T.proc.params(arg: T.untyped).returns(T.untyped)).returns(T.untyped)}
  def then(&blk); end
end
