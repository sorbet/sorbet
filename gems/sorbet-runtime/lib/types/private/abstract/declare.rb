# frozen_string_literal: true
# typed: true

module T::Private::Abstract::Declare
  Abstract = T::Private::Abstract
  AbstractUtils = T::AbstractUtils

  def self.declare_abstract(mod, type:)
    if AbstractUtils.abstract_module?(mod)
      raise "#{mod.name} is already declared as abstract"
    end

    Abstract::Data.set(mod, :can_have_abstract_methods, true)
    Abstract::Data.set(mod.singleton_class, :can_have_abstract_methods, true)
    Abstract::Data.set(mod, :abstract_type, type)

    mod.extend(Abstract::Hooks)
    mod.extend(T::InterfaceWrapper::Helpers)

    if mod.is_a?(Class)
      if mod < T::Struct
        raise "#{mod.name} is a subclass of T::Struct and cannot be declared abstract"
      end

      if type == :interface
        # Since `interface!` is just `abstract!` with some extra validation, we could technically
        # allow this, but it's unclear there are good use cases, and it might be confusing.
        raise "Classes can't be interfaces. Use `abstract!` instead of `interface!`."
      end

      if mod.instance_method(:initialize).owner == mod
        raise "You must call `abstract!` *before* defining an initialize method"
      end

      mod.send(:define_method, :initialize) do |*args, &blk|
        if self.class == mod
          raise "#{mod} is declared as abstract; it cannot be instantiated"
        end
        super(*args, &blk)
      end
    end
  end
end
