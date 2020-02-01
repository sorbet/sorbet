# frozen_string_literal: true
# typed: strict

module T::Props
  module Private
    module DeserializeHandleNil
      extend T::Sig

      TrustedRuby = RubyGen::TrustedRuby

      class RaiseOnNil < RubyGen::Template
        sig {params(serialized_form: String).returns(TrustedRuby).checked(:never)}
        def self.from_serialized_form(serialized_form)
          new(serialized_form: RubyGen::StringLiteral.new(serialized_form)).generate
        end

        sig {override.returns(String).checked(:never)}
        def self.format_string
          'self.class.decorator.raise_nil_deserialize_error(%<serialized_form>s)'
        end
      end

      class StoreOnNil < RubyGen::Template
        sig {params(prop: Symbol).returns(TrustedRuby).checked(:never)}
        def self.from_prop(prop)
          new(prop: RubyGen::SymbolLiteral.new(prop)).generate
        end

        sig {override.returns(String).checked(:never)}
        def self.format_string
          'required_prop_missing_from_deserialize(%<prop>s)'
        end
      end

      class ApplyDefaultDynamically < RubyGen::Template
        sig {params(prop: Symbol).returns(TrustedRuby).checked(:never)}
        def self.from_prop(prop)
          new(prop: RubyGen::SymbolLiteral.new(prop)).generate
        end

        sig {override.returns(String).checked(:never)}
        def self.format_string
          'self.class.decorator.props_with_defaults.fetch(%<prop>s).default'
        end
      end

      module Default
        TRUE = T.let(TrustedRuby.constant('true'), TrustedRuby)
        FALSE = T.let(TrustedRuby.constant('false'), TrustedRuby)
        NIL = T.let(TrustedRuby.constant('nil'), TrustedRuby)
        EMPTY_ARRAY = T.let(TrustedRuby.constant('[]'), TrustedRuby)
        EMPTY_HASH = T.let(TrustedRuby.constant('{}'), TrustedRuby)
      end

      # This is very similar to what we do in ApplyDefault, but has a few
      # key differences that mean we don't just re-use the code:
      #
      # 1. Where the logic in construction is that we generate a default
      #    if & only if the prop key isn't present in the input, here we'll
      #    generate a default even to override an explicit nil, but only
      #    if the prop is actually required.
      # 2. Since we're generating raw Ruby source, we can remove a layer
      #    of indirection for marginally better performance; this seems worth
      #    it for the common cases of literals and empty arrays/hashes.
      # 3. We need to care about the distinction between `raise_on_nil_write`
      #    and actually non-nilable, where new-instance construction doesn't.
      #
      # So we fall back to ApplyDefault only when one of the cases just
      # mentioned doesn't apply.
      sig do
        params(
          prop: Symbol,
          serialized_form: String,
          default: T.nilable(ApplyDefault),
          nilable_type: T::Boolean,
          raise_on_nil_write: T::Boolean,
        )
        .returns(T.any(TrustedRuby, RubyGen::TemplateVar))
        .checked(:never)
      end
      def self.generate(prop:, serialized_form:, default:, nilable_type:, raise_on_nil_write:)
        if !nilable_type
          case default
          when NilClass
            RaiseOnNil.from_serialized_form(serialized_form)
          when ApplyPrimitiveDefault
            literal = default.default
            case literal
            when Integer
              RubyGen::IntegerLiteral.new(literal)
            when Float
              RubyGen::FloatLiteral.new(literal)
            when TrueClass
              Default::TRUE
            when FalseClass
              Default::FALSE
            when String
              RubyGen::StringLiteral.new(literal)
            when Symbol
              RubyGen::SymbolLiteral.new(literal)
            when NilClass
              Default::NIL
            else
              ApplyDefaultDynamically.from_prop(prop)
            end
          when ApplyEmptyArrayDefault
            Default::EMPTY_ARRAY
          when ApplyEmptyHashDefault
            Default::EMPTY_HASH
          else
            ApplyDefaultDynamically.from_prop(prop)
          end
        elsif raise_on_nil_write
          StoreOnNil.from_prop(prop)
        else
          Default::NIL
        end
      end
    end
  end
end
