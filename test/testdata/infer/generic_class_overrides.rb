# typed: true

class Parent
  extend T::Sig
  extend T::Generic
  abstract!

  # TODO(jez) This only tests Elem, which is an invariant type member. Also test with co- and contravariant
  # TODO(jez) The `T.proc` cases test AppliedType's applied to a contravariant generic, but there are no tests of applying a covariant generic (maybe enough to just test with T.proc that returns something?).
  # TODO(jez) Do "module instance method overriden by class singleton method" to be sure that resultTypeAsSeenFrom works
  Elem = type_member

  sig {abstract.returns(Elem)}
  def example_returns_1; end
  sig {abstract.returns(Elem)}
  def example_returns_2; end

  sig {abstract.params(x: Elem).void}
  def example_params_1(x); end
  sig {abstract.params(x: Elem).void}
  def example_params_2(x); end

  sig {abstract.params(blk: T.proc.params(x: Elem).void).void}
  def example_proc_1(&blk); end
  sig {abstract.params(blk: T.proc.params(x: Elem).void).void}
  def example_proc_2(&blk); end

  sig {abstract.params(x: T.all(Elem, Kernel)).void}
  def example_params_all_1(x); end
  sig {abstract.params(x: T.all(Elem, Kernel)).void}
  def example_params_all_2(x); end
  sig {abstract.params(x: T.all(Elem, Kernel)).void}
  def example_params_all_3(x); end

  sig {abstract.type_parameters(:U).params(x: T.type_parameter(:U)).void}
  def example_type_param_1(x); end
  sig {abstract.type_parameters(:U).params(x: T.type_parameter(:U)).void}
  def example_type_param_2(x); end

  sig {abstract.type_parameters(:U).params(x: Kernel).void}
  def example_type_param_child_1(x); end
  sig {abstract.type_parameters(:U).params(x: Kernel).void}
  def example_type_param_child_2(x); end
end

class Child < Parent
  Elem = type_member

  sig {override.returns(T.nilable(Elem))}
  def example_returns_1; end
  sig {override.returns(T.all(Elem, Kernel))}
  def example_returns_2; raise; end

  sig {override.params(x: T.nilable(Elem)).void}
  def example_params_1(x); end
  sig {override.params(x: T.all(Elem, Kernel)).void}
  def example_params_2(x); end

  sig {override.params(blk: T.proc.params(x: T.nilable(Elem)).void).void}
  def example_proc_1(&blk); end
  sig {override.params(blk: T.proc.params(x: T.all(Elem, Kernel)).void).void}
  def example_proc_2(&blk); end

  sig {override.params(x: Elem).void}
  def example_params_all_1(x); end
  sig {override.params(x: Kernel).void}
  def example_params_all_2(x); end
  sig {override.params(x: Integer).void}
  def example_params_all_3(x); end

  sig {override.type_parameters(:U).params(x: T.any(T.type_parameter(:U), Integer)).void}
  def example_type_param_1(x); end
  sig {override.type_parameters(:U).params(x: Integer).void}
  def example_type_param_2(x); end

  sig {override.type_parameters(:U).params(x: T.any(Kernel, T.type_parameter(:U))).void}
  def example_type_param_child_1(x); end
  sig {override.type_parameters(:U).params(x: T.all(Kernel, T.type_parameter(:U))).void}
  def example_type_param_child_2(x); end
end
