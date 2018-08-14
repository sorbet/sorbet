# typed: true
class BasicObject
  Sorbet.sig.returns(T.any(TrueClass, FalseClass))
  def !(); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def !=(other); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def ==(other); end

  Sorbet.sig(
      arg0: Symbol,
      arg1: BasicObject,
  )
  .returns(T.untyped)
  def __send__(arg0, *arg1); end

  Sorbet.sig(
      other: BasicObject,
  )
  .returns(T.any(TrueClass, FalseClass))
  def equal?(other); end

  Sorbet.sig(
      arg0: String,
      filename: String,
      lineno: Integer,
  )
  .returns(T.untyped)
  Sorbet.sig(
      blk: T.proc().returns(BasicObject),
  )
  .returns(T.untyped)
  def instance_eval(arg0=_, filename=_, lineno=_, &blk); end

  Sorbet.sig(
      args: BasicObject,
      blk: BasicObject,
  )
  .returns(T.untyped)
  def instance_exec(*args, &blk); end

  Sorbet.sig.returns(Integer)
  def __id__(); end
end
