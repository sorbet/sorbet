# typed: __STDLIB_INTERNAL

class Method < Object
  sig {returns(Proc)}
  def to_proc; end

  sig {params(args: T.untyped).returns(T.untyped)}
  def call(*args);end

  sig {params(_: T.untyped).returns(T.untyped)}
  def <<(_); end

  sig {params(_: T.untyped).returns(T.untyped)}
  def ===(*_); end

  sig {params(_: T.untyped).returns(T.untyped)}
  def >>(_); end

  sig {params(_: T.untyped).returns(T.untyped)}
  def [](*_); end

  sig {returns(Integer)}
  def arity; end

  sig {returns(Method)}
  def clone; end

  sig {params(_: T.untyped).returns(T.untyped)}
  def curry(*_); end

  sig {returns(Symbol)}
  def name; end

  sig {returns(Symbol)}
  def original_name; end

  sig {returns(T.any(Class, Module))}
  def owner; end

  sig {returns(T::Array[T.untyped])}
  def parameters; end

  sig {returns(T.untyped)}
  def receiver; end

  sig {returns(T.untyped)}
  def source_location; end

  sig {returns(T.nilable(Method))}
  def super_method; end

  sig {returns(T.untyped)}
  def unbind; end
end
