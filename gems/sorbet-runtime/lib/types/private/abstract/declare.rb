# frozen_string_literal: true
# typed: true

module T::Private::Abstract::Declare
  Abstract = T::Private::Abstract
  AbstractUtils = T::AbstractUtils

  HAS_ATTACHED_OBJECT = RUBY_VERSION >= "3.3.0"

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

    if mod.is_a?(Class)
      if type == :interface
        # Since `interface!` is just `abstract!` with some extra validation, we could technically
        # allow this, but it's unclear there are good use cases, and it might be confusing.
        raise "Classes can't be interfaces. Use `abstract!` instead of `interface!`."
      end

      if Object.instance_method(:method).bind_call(mod, :new).owner == mod
        raise "You must call `abstract!` *before* defining a `new` method"
      end

      # Don't need to silence warnings via without_ruby_warnings when calling
      # define_method because of the guard above

      mod.send(:define_singleton_method, :new) do |*args, &blk|
        result = super(*args, &blk)
        if result.instance_of?(mod)
          raise "#{mod} is declared as abstract; it cannot be instantiated"
        end

        # Don't even bother if we can't go from the singleton class to the attached class.
        if !HAS_ATTACHED_OBJECT
          return result
        end

        # See if we were called on a module that resolved to the abstract override, and
        # bypass the abstract override for the next call.
        me = self.method(:new)
        loc = me.source_location&.[](0)
        # If the first resolved method on `self` is not in this file, then we can't apply
        # this optimization, because the `new` that we're now in is somewhere in the
        # hierarchy above the first resolved method.
        if loc != __FILE__
          return result
        end

        while loc == __FILE__
          # This method must exist, or we would have errored in the earlier `super` call.
          me = T.must(me.super_method)
          loc = me.source_location&.[](0)
        end

        # TODO(froydnj) we should be able to ladder up the method inheritance
        # chain to see if there are multiple abstract .new methods and resolve
        # to the farthest-away non-abstract one.
        T::Private::DeclState.current.without_on_method_added do
          self.send(:define_singleton_method, :new, me.unbind)
        end
        result
      end

      # Ruby doesn not emit "method redefined" warnings for aliased methods
      # (more robust than undef_method that would create a small window in which the method doesn't exist)
      mod.singleton_class.send(:alias_method, :new, :new)

      if mod.singleton_class.respond_to?(:ruby2_keywords, true)
        mod.singleton_class.send(:ruby2_keywords, :new)
      end
    end
  end
end
