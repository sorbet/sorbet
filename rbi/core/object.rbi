# typed: true

class BasicObject
  sig {returns(T::Boolean)}
  def !(); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def !=(other); end

  sig do
    params(
        other: BasicObject,
    )
    .returns(T::Boolean)
  end
  def ==(other); end

  sig {returns(Integer)}
  def __id__(); end

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
    .returns(T::Boolean)
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
    type_parameters(:U)
    .params(
        blk: T.proc.bind(T.untyped).params().returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def instance_eval(arg0=T.unsafe(nil), filename=T.unsafe(nil), lineno=T.unsafe(nil), &blk); end

  sig do
    type_parameters(:U, :V)
    .params(
        args: T.type_parameter(:V),
        blk: T.proc.bind(T.untyped).params(args: T.untyped).returns(T.type_parameter(:U)),
    )
    .returns(T.type_parameter(:U))
  end
  def instance_exec(*args, &blk); end
end

class Object < BasicObject
  sig {returns(Integer)}
  def object_id(); end
end
