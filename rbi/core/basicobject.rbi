# typed: true
class BasicObject
  sig {returns(T.any(TrueClass, FalseClass))}
  def !(); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def !=(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def ==(other); end

  sig do
    params(
        arg0: Symbol,
        arg1: BasicObject,
    )
    .returns(T.untyped)
  end
  def __send__(arg0, *arg1); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T.any(TrueClass, FalseClass))
  end
  def equal?(other); end

  sig do
    params(
        arg0: String,
        filename: String,
        lineno: Integer,
    )
    .returns(T.untyped)
  end
  sig do
    params(
        blk: T.proc.params().returns(BasicObject),
    )
    .returns(T.untyped)
  end
  def instance_eval(arg0=T.unsafe(nil), filename=T.unsafe(nil), lineno=T.unsafe(nil), &blk); end

  sig do
    params(
        args: BasicObject,
        blk: BasicObject,
    )
    .returns(T.untyped)
  end
  def instance_exec(*args, &blk); end

  sig {returns(Integer)}
  def __id__(); end
end
