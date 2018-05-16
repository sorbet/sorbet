# typed: true
class BasicObject
  sig.returns(T.any(TrueClass, FalseClass))
  def !(); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def !=(other); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(other); end

  sig(
      arg0: Symbol,
      arg1: BasicObject,
  )
  .returns(T.untyped)
  def __send__(arg0, *arg1); end

  sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(other); end

  sig(
      arg0: String,
      filename: String,
      lineno: Integer,
  )
  .returns(T.untyped)
  sig(
      blk: T.proc().returns(BasicObject),
  )
  .returns(T.untyped)
  def instance_eval(arg0=_, filename=_, lineno=_, &blk); end

  sig(
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(T.untyped)
  def instance_exec(*args, &blk); end

  sig.returns(Integer)
  def __id__(); end
end
