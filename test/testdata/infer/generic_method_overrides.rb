# typed: true

module IFoo; end

class Parent
  extend T::Sig
  sig do
    overridable
      .type_parameters(:U)
      .params(x: T.type_parameter(:U))
      .returns(T.type_parameter(:U))
  end
  def id(x); x; end

  sig do
    overridable
      .type_parameters(:U)
      .params(
        x: T.type_parameter(:U),
        f: T.proc.params(x: T.type_parameter(:U)).void,
      )
      .void
  end
  def apply_f(x, f); f.call(x); end
end

class ChildGood < Parent
  extend T::Sig
  sig do
    override
      .type_parameters(:V)
      .params(x: T.nilable(T.type_parameter(:V)))
      .returns(T.all(T.type_parameter(:V), IFoo))
  end
  def id(x)
    case x
    when nil then raise
    when IFoo then x
    else raise
    end
  end

  sig do
    override
      .type_parameters(:V)
      .params(
        x: T.nilable(T.type_parameter(:V)),
        f: T.proc.params(x: T.all(T.type_parameter(:V), IFoo)).void
      )
      .void
  end
  def apply_f(x, f)
    case x
    when nil then raise
    when IFoo then f.call(x)
    else raise
    end
  end
end

class ChildBad < Parent
  extend T::Sig
  sig do
    override
      .type_parameters(:W)
      .params(x: T.all(T.type_parameter(:W), IFoo))
      .returns(T.nilable(T.type_parameter(:W)))
  end
  def id(x); x; end
# ^^^^^^^^^ error: Parameter `x` of type `T.all(IFoo, T.type_parameter(:W))` not compatible with type of overridable method `Parent#id`
# ^^^^^^^^^ error: Return type `T.nilable(T.type_parameter(:W))` does not match return type of overridable method `Parent#id`

  sig do
    override
      .type_parameters(:W)
      .params(
        x: T.all(T.type_parameter(:W), IFoo),
        f: T.proc.params(x: T.nilable(T.type_parameter(:W))).void
      )
      .void
  end
  def apply_f(x, f)
# ^^^^^^^^^^^^^^^^^ error: Parameter `x` of type `T.all(IFoo, T.type_parameter(:W))` not compatible with type of overridable method `Parent#apply_f`
# ^^^^^^^^^^^^^^^^^ error: Parameter `f` of type `T.proc.params(arg0: T.nilable(T.type_parameter(:W))).void` not compatible with type of overridable method `Parent#apply_f`
    f.call(x)
  end
end

class ChildNoSig < Parent
  def id(x); x; end
  def apply_f(x, f); f.call(x); end
end
