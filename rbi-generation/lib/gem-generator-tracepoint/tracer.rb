# frozen_string_literal: true
# typed: true

require_relative './module_utils'

require 'set'

alias DelegateClass_without_rbi_generator DelegateClass
def DelegateClass(superclass)
  result = DelegateClass_without_rbi_generator(superclass)
  SorbetRBIGeneration::GemGeneratorTracepoint::Tracer.register_delegate_class(superclass, result)
  result
end

module SorbetRBIGeneration 
  module GemGeneratorTracepoint 
    class Tracer
      module ModuleOverride
        def include(mod, *smth)
          result = super
          SorbetRBIGeneration::GemGeneratorTracepoint::Tracer.module_included(mod, self)
          result
        end
      end
      Module.prepend(ModuleOverride)

      module ObjectOverride
        def extend(mod, *args)
          result = super
          SorbetRBIGeneration::GemGeneratorTracepoint::Tracer.module_extended(mod, self)
          result
        end
      end
      Object.prepend(ObjectOverride)

      module ClassOverride
        def new(*)
          result = super
          SorbetRBIGeneration::GemGeneratorTracepoint::Tracer.module_created(result)
          result
        end
      end
      Class.prepend(ClassOverride)

      def self.register_delegate_class(klass, delegate)
        @delegate_classes[ModuleUtils.real_object_id(delegate)] = klass
      end

      def self.module_created(mod)
        add_to_context(type: :module, module: mod)

        # Record the class as a subclass so we don't accidentally pick it up
        # when processing Class#inherited in next require
        if ModuleUtils.real_is_a?(mod, Class)
          @subclasses[ModuleUtils.real_object_id(mod.superclass)] ||= Set.new
          @subclasses[ModuleUtils.real_object_id(mod.superclass)] << ModuleUtils.real_object_id(mod)
        end
      end

      def self.module_included(included, includer)
        add_to_context(type: :include, module: includer, include: included)
      end

      def self.module_extended(extended, extender)
        add_to_context(type: :extend, module: extender, extend: extended)
      end

      def self.method_added(mod, method, singleton)
        add_to_context(type: :method, module: mod, method: method, singleton: singleton)
      end

      def self.module_inherited(mod)
        add_to_context(type: :inherited, module: mod)
      end

      Sorbet.sig {returns({files: T::Hash, delegate_classes: T::Hash})}
      def self.trace
        start
        yield
        finish
        trace_results
      end

      Sorbet.sig {void}
      def self.start
        pre_cache_module_methods
        install_tracepoints
      end

      Sorbet.sig {void}
      def self.finish
        disable_tracepoints
      end

      Sorbet.sig {returns({files: T::Hash, delegate_classes: T::Hash})}
      def self.trace_results
        {
          files: @files,
          delegate_classes: @delegate_classes
        }
      end

      private

      @modules = {}
      @context_stack = [[]]
      @files = {}
      @delegate_classes = {}
      @subclasses = {}

      def self.pre_cache_module_methods
        ObjectSpace.each_object(Module) do |mod_|
          mod = T.cast(mod_, Module)
          @modules[ModuleUtils.real_object_id(mod)] = (mod.instance_methods(false) + mod.private_instance_methods(false)).to_set
          new_subclasses(mod_)
        end
      end

      def self.add_to_context(item)
        # The stack can be empty because we start the :c_return TracePoint inside a 'require' call.
        # In this case, it's okay to simply add something to the stack; it will be popped off when
        # the :c_return is traced.
        @context_stack << [] if @context_stack.empty?
        @context_stack.last << item
      end

      def self.install_tracepoints
        @class_tracepoint = TracePoint.new(:class) do |tp|
          module_created(tp.self)
        end
        @c_call_tracepoint = TracePoint.new(:c_call) do |tp|
          case tp.method_id
          when :inherited
            module_inherited(tp.self)
          when :require, :require_relative
            @context_stack << []
          end
        end
        @c_return_tracepoint = TracePoint.new(:c_return) do |tp|
          case tp.method_id
          when :require, :require_relative
            popped = @context_stack.pop

            next if popped.empty?

            path = $LOADED_FEATURES.last
            if tp.return_value != true # intentional true check
              next if popped.size == 1 && popped[0][:module].is_a?(LoadError)
              warn("Unexpected: constants or methods were defined when #{tp.method_id} didn't return true; adding to #{path} instead")
            end

            # raise 'Unexpected: constants or methods were defined without a file added to $LOADED_FEATURES' if path.nil?
            # raise "Unexpected: #{path} is already defined in files" if files.key?(path)

            process_inherited(popped, path)

            @files[path] ||= []
            @files[path] += popped

          # popped.each { |item| item[:path] = path }
          when :method_added, :singleton_method_added
            begin
              tp.disable

              singleton = tp.method_id == :singleton_method_added
              receiver = singleton ? tp.self.singleton_class : tp.self
              methods = receiver.instance_methods(false) + receiver.private_instance_methods(false)
              set = @modules[ModuleUtils.real_object_id(receiver)] ||= Set.new
              added = methods.find { |m| !set.include?(m) }
              if added.nil?
                # warn("Warning: could not find method added to #{tp.self} at #{tp.path}:#{tp.lineno}")
                next
              end
              set << added

              method_added(tp.self, added, singleton)
            ensure
              tp.enable
            end
          end
        end
        @class_tracepoint.enable
        @c_call_tracepoint.enable
        @c_return_tracepoint.enable
      end

      def self.disable_tracepoints
        @class_tracepoint.disable
        @c_call_tracepoint.disable
        @c_return_tracepoint.disable
      end

      def self.process_inherited(context, path)
        inherited = Set.new
        context.reject! do |item|
          next false unless item[:type] == :inherited
          inherited << ModuleUtils.real_object_id(item[:module])
          true
        end

        # Inherited classes defined in Ruby .rb source files are detected using the
        # Class.new and :class Tracepoints. Classes that are created in C extensions
        # (required from .bundle files) are not. So we detect any new classes below
        # and add them as modules to the current context.
        return if path =~ /\.rb$/
        inherited.each do |object_id|
          superclass = ObjectSpace._id2ref(object_id)
          subclasses = new_subclasses(superclass)
          subclasses.each do |subclass|
            context << { type: :module, module: subclass }
          end
        end
      end

      def self.new_subclasses(klass)
        previous = @subclasses[ModuleUtils.real_object_id(klass)] ||= Set.new
        added = []
        ObjectSpace.each_object(klass.singleton_class) do |k|
          if previous.add?(ModuleUtils.real_object_id(k))
            added << k if ModuleUtils.real_is_a?(k, Class) && T.cast(k, Class).superclass == klass
          end
        end
        added
      end

    end
  end
end

