# frozen_string_literal: true
# typed: false

module T::Props

  # Helper for generating methods that replace themselves with a specialized
  # version on first use. The main use case is when we want to generate a
  # method using the full set of props on a class; we can't do that during
  # prop definition because we have no way of knowing whether we are defining
  # the last prop.
  module HasLazilySpecializedMethods
    module DecoratorMethods
      extend T::Sig

      sig {returns(T::Hash[Symbol, T.proc.returns(Private::RubyGen::TrustedRuby)]).checked(:never)}
      private def lazily_defined_methods
        @lazily_defined_methods ||= {}
      end

      sig {params(name: Symbol).void}
      private def eval_lazily_defined_method!(name)
        source = lazily_defined_methods.fetch(name).call

        cls = decorated_class
        cls.class_eval(source.to_s)
        cls.send(:private, name)
      end

      sig {params(name: Symbol, blk: T.proc.returns(Private::RubyGen::TrustedRuby)).void}
      private def enqueue_lazy_method_definition!(name, &blk)
        lazily_defined_methods[name] = blk

        cls = decorated_class
        cls.define_method(name) do |*args|
          self.class.decorator.send(:eval_lazily_defined_method!, name)
          send(name, *args)
        end
        cls.send(:private, name)
      end

      sig {void}
      def eagerly_define_lazy_methods!
        source = lazily_defined_methods.values.map(&:call).map(&:to_s).join("\n\n")

        cls = decorated_class
        cls.class_eval(source)
        lazily_defined_methods.keys.each {|name| cls.send(:private, name)}
      end
    end
  end
end
