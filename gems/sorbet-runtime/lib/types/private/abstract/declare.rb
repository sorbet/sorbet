# frozen_string_literal: true
# typed: true

module T::Private::Abstract::Declare
  Abstract = T::Private::Abstract
  AbstractUtils = T::AbstractUtils

  def self.declare_abstract(mod, type:)
    if AbstractUtils.abstract_module?(mod)
      raise "#{mod} is already declared as abstract"
    end
    if T::Private::Final.final_module?(mod)
      raise "#{mod} was already declared as final and cannot be declared as abstract"
    end

    Abstract::Data.set(mod, :can_have_abstract_methods, true)
    Abstract::Data.set(mod.singleton_class, :can_have_abstract_methods, true)
    Abstract::Data.set(mod, :abstract_type, type)

    mod.extend(Abstract::Hooks)
    mod.extend(T::InterfaceWrapper::Helpers)

    if mod.is_a?(Class)
      if type == :interface
        # Since `interface!` is just `abstract!` with some extra validation, we could technically
        # allow this, but it's unclear there are good use cases, and it might be confusing.
        raise "Classes can't be interfaces. Use `abstract!` instead of `interface!`."
      end

      if mod.instance_method(:initialize).owner == mod
        raise "You must call `abstract!` *before* defining an initialize method"
      end

      # Don't need to silence warnings via without_ruby_warnings when calling
      # define_method because of the guard above

      mod.send(:define_method, :initialize) do |*args, &blk|
        if self.class == mod
          raise "#{mod} is declared as abstract; it cannot be instantiated"
        end
        super(*args, &blk)
      end

      # Ruby doesn not emit "method redefined" warnings for aliased methods
      # (more robust than undef_method that would create a small window in which the method doesn't exist)
      mod.send(:alias_method, :initialize, :initialize)

      if mod.respond_to?(:ruby2_keywords, true)
        mod.send(:ruby2_keywords, :initialize)
      end
    end
  end
end
