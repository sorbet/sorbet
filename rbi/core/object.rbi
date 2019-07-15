# typed: __STDLIB_INTERNAL

class Object < BasicObject
  include Kernel

  sig {returns(Integer)}
  def object_id(); end

  sig { params(blk: T.proc.params(arg: T.untyped).returns(T.untyped)).returns(T.untyped) }
  def yield_self(&blk); end

  alias_method :then, :yield_self
end
