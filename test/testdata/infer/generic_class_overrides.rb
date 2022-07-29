# typed: true

class Parent
  extend T::Sig
  extend T::Generic
  abstract!

  Elem = type_member

  sig {abstract.returns(Elem)}
  def example_returns_1; end
  sig {abstract.returns(Elem)}
  def example_returns_2; end

  sig {abstract.params(x: Elem).void}
  def example_params_1(x); end
  sig {abstract.params(x: Elem).void}
  def example_params_2(x); end

  sig {abstract.params(f: T.proc.params(x: Elem).void).void}
  def example_proc_1(f); end
  sig {abstract.params(f: T.proc.params(x: Elem).void).void}
  def example_proc_2(f); end

  sig {abstract.params(f: T.proc.returns(Elem)).void}
  def example_proc_returns_1(f); end
  sig {abstract.params(f: T.proc.returns(Elem)).void}
  def example_proc_returns_2(f); end

  sig {abstract.params(f: T.proc.params(x: Elem).void).void}
  def example_block_1(&f); end
  sig {abstract.params(f: T.proc.params(x: Elem).void).void}
  def example_block_2(&f); end

  sig {abstract.params(f: T.proc.returns(Elem)).void}
  def example_block_returns_1(&f); end
  sig {abstract.params(f: T.proc.returns(Elem)).void}
  def example_block_returns_2(&f); end

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
  def example_returns_1; end # error: Return type `T.nilable(Child::Elem)` does not match return type of abstract method `Parent#example_returns_1`
  sig {override.returns(T.all(Elem, Kernel))}
  def example_returns_2; raise; end

  sig {override.params(x: T.nilable(Elem)).void}
  def example_params_1(x); end
  sig {override.params(x: T.all(Elem, Kernel)).void}
  def example_params_2(x); end # error: Parameter `x` of type `T.all(Kernel, Child::Elem)` not compatible with type of abstract method `Parent#example_params_2`

  sig {override.params(f: T.proc.params(x: T.nilable(Elem)).void).void}
  def example_proc_1(f); end # error: Parameter `f` of type `T.proc.params(arg0: T.nilable(Child::Elem)).void` not compatible with type of abstract method `Parent#example_proc_1`
  sig {override.params(f: T.proc.params(x: T.all(Elem, Kernel)).void).void}
  def example_proc_2(f); end

  sig {override.params(f: T.proc.returns(T.nilable(Elem))).void}
  def example_proc_returns_1(f); end
  sig {override.params(f: T.proc.returns(T.all(Elem, Kernel))).void}
  def example_proc_returns_2(f); end # error: Parameter `f` of type `T.proc.returns(T.all(Kernel, Child::Elem))` not compatible with type of abstract method `Parent#example_proc_returns_2`

  sig {override.params(f: T.proc.params(x: T.nilable(Elem)).void).void}
  def example_block_1(&f); end # error: Block parameter `f` of type `T.proc.params(arg0: T.nilable(Child::Elem)).void` not compatible with type of abstract method `Parent#example_block_1`
  sig {override.params(f: T.proc.params(x: T.all(Elem, Kernel)).void).void}
  def example_block_2(&f); end

  sig {override.params(f: T.proc.returns(T.nilable(Elem))).void}
  def example_block_returns_1(&f); end
  sig {override.params(f: T.proc.returns(T.all(Elem, Kernel))).void}
  def example_block_returns_2(&f); end # error: Block parameter `f` of type `T.proc.returns(T.all(Kernel, Child::Elem))` not compatible with type of abstract method `Parent#example_block_returns_2`

  sig {override.params(x: Elem).void}
  def example_params_all_1(x); end
  sig {override.params(x: Kernel).void}
  def example_params_all_2(x); end
  sig {override.params(x: Integer).void}
  def example_params_all_3(x); end # error: Parameter `x` of type `Integer` not compatible with type of abstract method `Parent#example_params_all_3`

  sig {override.type_parameters(:U).params(x: T.any(T.type_parameter(:U), Integer)).void}
  def example_type_param_1(x); end
  sig {override.type_parameters(:U).params(x: Integer).void}
  def example_type_param_2(x); end # error: Parameter `x` of type `Integer` not compatible with type of abstract method `Parent#example_type_param_2`

  sig {override.type_parameters(:U).params(x: T.any(Kernel, T.type_parameter(:U))).void}
  def example_type_param_child_1(x); end
  sig {override.type_parameters(:U).params(x: T.all(Kernel, T.type_parameter(:U))).void}
  def example_type_param_child_2(x); end # error: Parameter `x` of type `T.all(Kernel, T.type_parameter(:U))` not compatible with type of abstract method `Parent#example_type_param_child_2`
end

module Runnable
  extend T::Sig
  extend T::Generic
  interface!
  Arg = type_member

  sig {abstract.returns(Arg)}
  def example_returns_1; end
  sig {abstract.returns(Arg)}
  def example_returns_2; end
  sig {abstract.returns(Arg)}
  def example_returns_3; end
end

class MyScript
  extend T::Sig
  extend T::Generic
  extend Runnable

  Arg = type_template

  sig {override.returns(T.nilable(Arg))}
  def self.example_returns_1; end # error: Return type `T.nilable(T.class_of(MyScript)::Arg)` does not match return type of abstract method `Runnable#example_returns_1`
  sig {override.returns(Arg)}
  def self.example_returns_2; raise; end
  sig {override.returns(T.all(Arg, Kernel))}
  def self.example_returns_3; raise; end
end

