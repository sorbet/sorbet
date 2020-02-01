# frozen_string_literal: true
# typed: strict

module T::Props
  module Private

    # Simple framework for generating Ruby source code from semi-trusted input
    # like application-specified prop names.
    #
    # The intent is that it should be hard to accidentally allow arbitrary code
    # generation while using this framework. If an adversary has the ability to
    # pass objects with arbitrarily overridden methods (e.g. `is_a?`) or to
    # arbitrarily monkeypatch Kernel, this provides no safety, but also no new
    # capabilities (they could simply call `*_eval` themselves). But we
    # want to reduce the likelihood that an application developer could
    # accidentally pass dangerous input such as a user-provided string or part
    # of an [RCE gadget chain][1] to `*_eval`.
    #
    # To implement this, we limit the ways to construct a TrustedRuby instance
    # such that any instance should be built from sanitized input combined in a
    # planned way. As part of this, unlike most of the T::Props internals, we
    # use runtime typechecking (mostly via `T.let`).
    #
    # We could get arguably stronger guarantees by using something like the
    # `unparser` gem, but that is, unfortunately, very slow as of writing,
    # and anyway also relies on, e.g., `inspect` doing [safe escaping][2].
    #
    # [1] https://www.elttam.com//blog/ruby-deserialization/
    # [2] https://github.com/mbj/unparser/blob/6d21163c2af2993d36ef01ec8f3fa924f09e93f4/lib/unparser/emitter/literal/primitive.rb#L26
    module RubyGen

      class TrustedRuby
        extend T::Sig
        extend T::Helpers
        sealed!

        sig {params(src: String).void.checked(:never)}
        def initialize(src)
          @src = T.let(src, String)
        end

        sig {override.returns(String).checked(:never)}
        def to_s
          @src
        end

        sig {params(srcs: T::Array[TrustedRuby]).returns(TrustedRuby)}
        def self.join(srcs)
          new(srcs.map(&:to_s).join)
        end

        # This should only be called with statically defined strings,
        # not with anything generated at runtime. TODO: Check this with
        # Rubocop.
        sig {params(src: String).returns(TrustedRuby)}
        def self.constant(src)
          if !src.frozen?
            raise ArgumentError.new("Expected frozen constant input")
          else
            new(src)
          end
        end

        sig {params(template: Template).returns(TrustedRuby)}
        def self.eval_template(template)
          new(Kernel.format(template.class.format_string, template.vars))
        end

        # Only construct using methods above
        private_class_method :new
      end

      class TemplateVar
        extend T::Sig
        extend T::Helpers
        abstract!
        sealed!
      end

      class SymbolLiteral < TemplateVar
        # checked(:never) - checked by T.let
        sig {params(sym: Symbol).void.checked(:never)}
        def initialize(sym)
          @sym = T.let(sym, Symbol)
        end

        # checked(:never) - trivial & hot
        sig {override.returns(String).checked(:never)}
        def to_s
          @sym.inspect
        end
      end

      class StringLiteral < TemplateVar
        # checked(:never) - checked by T.let
        sig {params(str: String).void.checked(:never)}
        def initialize(str)
          @str = T.let(str.freeze, String)
        end

        # checked(:never) - trivial & hot
        sig {override.returns(String).checked(:never)}
        def to_s
          @str.inspect
        end
      end

      class IntegerLiteral < TemplateVar
        # checked(:never) - checked by T.let
        sig {params(i: Integer).void.checked(:never)}
        def initialize(i)
          @i = T.let(i, Integer)
        end

        # checked(:never) - trivial & hot
        sig {override.returns(String).checked(:never)}
        def to_s
          @i.to_s
        end
      end

      class FloatLiteral < TemplateVar
        # checked(:never) - checked by T.let
        sig {params(f: Float).void.checked(:never)}
        def initialize(f)
          @f = T.let(f, Float)
        end

        # checked(:never) - trivial & hot
        sig {override.returns(String).checked(:never)}
        def to_s
          @f.to_s
        end
      end

      class InstanceVar < TemplateVar
        # checked(:never) - checked by T.let
        sig {params(name: Symbol).void.checked(:never)}
        def initialize(name)
          if !name.match?(/\A@[a-zA-Z0-9_]+\z/)
            raise ArgumentError.new("Invalid instance variable name: #{name}")
          end
          @name = T.let(name, Symbol)
        end

        # checked(:never) - trivial & hot
        sig {override.returns(String).checked(:never)}
        def to_s
          @name.to_s
        end
      end

      class ModuleLiteral < TemplateVar
        # checked(:never) - checked by T.let
        sig {params(cls: Module).void.checked(:never)}
        def initialize(cls)
          @cls = T.let(cls, Module)
        end

        # Reasonable code does occasionally override the `name` method on
        # classes, so let's be robust to that.
        NAME = T.let(Module.instance_method(:name).freeze, UnboundMethod)

        # checked(:never) - trivial & hot
        sig {override.returns(String).checked(:never)}
        def to_s
          '::' + T.let(NAME.bind(@cls).call, String)
        end
      end

      # Abstract template. Subclasses should define a static format_string
      # which uses named parameters corresponding to the named keyword
      # arguments to `initialize`.
      class Template
        extend T::Sig
        extend T::Helpers
        abstract!

        # checked(:never) - checked in constructor
        sig(:final) {returns(T::Hash[Symbol, T.any(TemplateVar, TrustedRuby)]).checked(:never)}
        attr_reader :vars

        sig {abstract.returns(String)}
        def self.format_string; end

        # checked(:never) - checked by T.let
        sig {params(vars: T::Hash[Symbol, T.any(TemplateVar, TrustedRuby)]).void.checked(:never)}
        def initialize(vars)
          @vars = T.let(vars.freeze, T::Hash[Symbol, T.any(TemplateVar, TrustedRuby)])
        end

        # checked(:never) - checked by eval_template
        sig(:final) {returns(TrustedRuby).checked(:never)}
        def generate
          TrustedRuby.eval_template(self)
        end

        # Inheritors should provide factory methods with more specific params
        private_class_method :new
      end
    end
  end
end
