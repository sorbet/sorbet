# frozen_string_literal: true
# typed: true

require_relative '../real_stdlib'

require 'set'

if defined?(DelegateClass)
  alias DelegateClass_without_rbi_generator DelegateClass
  def DelegateClass(superclass)
    result = DelegateClass_without_rbi_generator(superclass)
    Sorbet::Private::GemGeneratorTracepoint::Tracer.register_delegate_class(superclass, result)
    result
  end
end

module Sorbet::Private
  module GemGeneratorTracepoint
    class Tracer
      module ModuleOverride
        def include(mod, *smth)
          result = super
          Sorbet::Private::GemGeneratorTracepoint::Tracer.on_module_included(mod, self)
          result
        end
      end
      Module.prepend(ModuleOverride)

      module ObjectOverride
        def extend(mod, *args)
          result = super
          Sorbet::Private::GemGeneratorTracepoint::Tracer.on_module_extended(mod, self)
          result
        end
      end
      Object.prepend(ObjectOverride)

      module ClassOverride
        def new(*)
          result = super
          Sorbet::Private::GemGeneratorTracepoint::Tracer.on_module_created(result)
          result
        end

        # This is a hack due to changes in kwargs with Ruby 2.7 and 3.0. Using
        # `*` for method delegation is deprecated in Ruby 2.7 and doesn't work
        # in Ruby 3.0.
        # See the "compatible delegation" section in this blog post:
        # https://www.ruby-lang.org/en/news/2019/12/12/separation-of-positional-and-keyword-arguments-in-ruby-3-0/
        #
        # Once Sorbet supports exclusively 2.7+ we can remove ruby2_keywords and
        # use the `...` delegation syntax instead.
        send(:ruby2_keywords, :new) if respond_to?(:ruby2_keywords, true)
      end
      Class.prepend(ClassOverride)

      def self.register_delegate_class(klass, delegate)
        @delegate_classes[Sorbet::Private::RealStdlib.real_object_id(delegate)] = klass
      end

      def self.on_module_created(mod)
        add_to_context(type: :module, module: mod)
      end

      def self.on_module_included(included, includer)
        add_to_context(type: :include, module: includer, include: included)
      end

      def self.on_module_extended(extended, extender)
        add_to_context(type: :extend, module: extender, extend: extended)
      end

      def self.on_method_added(mod, method, singleton)
        add_to_context(type: :method, module: mod, method: method, singleton: singleton)
      end

      T::Sig::WithoutRuntime.sig {returns({files: T::Hash, delegate_classes: T::Hash})}
      def self.trace
        start
        yield
        finish
        trace_results
      end

      T::Sig::WithoutRuntime.sig {void}
      def self.start
        pre_cache_module_methods
        install_tracepoints
      end

      T::Sig::WithoutRuntime.sig {void}
      def self.finish
        disable_tracepoints
      end

      T::Sig::WithoutRuntime.sig {returns({files: T::Hash, delegate_classes: T::Hash})}
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

      def self.pre_cache_module_methods
        ObjectSpace.each_object(Module) do |mod_|
          mod = T.cast(mod_, Module)
          @modules[Sorbet::Private::RealStdlib.real_object_id(mod)] = (Sorbet::Private::RealStdlib.real_instance_methods(mod, false) + Sorbet::Private::RealStdlib.real_private_instance_methods(mod, false)).to_set
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
          on_module_created(tp.self)
        end
        @c_call_tracepoint = TracePoint.new(:c_call) do |tp|

          # older version of JRuby unfortunately returned a String
          case tp.method_id.to_sym
          when :require, :require_relative
            @context_stack << []
          end
        end
        @c_return_tracepoint = TracePoint.new(:c_return) do |tp|

          # older version of JRuby unfortunately returned a String
          method_id_sym = tp.method_id.to_sym
          case method_id_sym
          when :require, :require_relative
            popped = @context_stack.pop

            next if popped.empty?

            path = $LOADED_FEATURES.last
            if tp.return_value != true # intentional true check
              next if popped.size == 1 && popped[0][:module].is_a?(LoadError)
              # warn("Unexpected: constants or methods were defined when #{tp.method_id} didn't return true; adding to #{path} instead")
            end

            # raise 'Unexpected: constants or methods were defined without a file added to $LOADED_FEATURES' if path.nil?
            # raise "Unexpected: #{path} is already defined in files" if files.key?(path)

            @files[path] ||= []
            @files[path] += popped

          # popped.each { |item| item[:path] = path }
          when :method_added, :singleton_method_added
            begin
              tp.disable

              singleton = method_id_sym == :singleton_method_added
              receiver = singleton ? Sorbet::Private::RealStdlib.real_singleton_class(tp.self) : tp.self

              # JRuby the main Object is not a module
              # so lets skip it, otherwise RealStdlib#real_instance_methods raises an exception since it expects one.
              next unless receiver.is_a?(Module)

              methods = Sorbet::Private::RealStdlib.real_instance_methods(receiver, false) + Sorbet::Private::RealStdlib.real_private_instance_methods(receiver, false)
              set = @modules[Sorbet::Private::RealStdlib.real_object_id(receiver)] ||= Set.new
              added = methods.find { |m| !set.include?(m) }
              if added.nil?
                # warn("Warning: could not find method added to #{tp.self} at #{tp.path}:#{tp.lineno}")
                next
              end
              set << added

              on_method_added(tp.self, added, singleton)
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
    end
  end
end
