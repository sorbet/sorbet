# frozen_string_literal: true
# typed: false

module T::Private::Final
  def self.declare(mod)
    if !mod.is_a?(Module)
      raise "#{mod.name} is not a module or method and cannot be declared as final"
    end
    if final_module?(mod)
      raise "#{mod.name} was already declared as final and cannot be re-declared as final"
    end
    if T::AbstractUtils.abstract_module?(mod)
      raise "#{mod.name} was already declared as abstract and cannot be declared as final"
    end
    if mod.is_a?(Class)
      orig_inherited = T::Private::ClassUtils.replace_method(mod.singleton_class, :inherited) do |arg|
        orig_inherited.bind(self).call(arg)
        raise "#{mod.name} was declared as final and cannot be inherited from"
      end
    end
    mark_as_final_module(mod)
    mark_as_final_module(mod.singleton_class)
    T::Private::Methods.install_hooks(mod)
  end

  def self.final_module?(mod)
    mod.instance_variable_defined?(:@sorbet_final_module) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
  end

  private_class_method def self.mark_as_final_module(mod)
    mod.instance_variable_set(:@sorbet_final_module, true) # rubocop:disable PrisonGuard/NoLurkyInstanceVariableAccess
    T::Private::Methods.add_module_with_final(mod)
  end
end
