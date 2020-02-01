# frozen_string_literal: true
# typed: strict

module T::Props
  module Private

    # Generates a specialized `serialize` implementation for a subclass of
    # T::Props::Serializable, using the facilities in `RubyGen`.
    #
    # The basic idea is that we analyze the props and for each prop, generate
    # the simplest possible logic as a block of Ruby source, so that we don't
    # pay the cost of supporting types like T:::Hash[CustomType, SubstructType]
    # when serializing a simple Integer. Then we join those together,
    # with a little shared logic to be able to detect when we get input keys
    # that don't match any prop.
    #
    # Each instance of this class is responsible for generating the Ruby source
    # for a single prop; there's a class method which takes a hash of props
    # and generates the full `serialize` implementation.
    module SerializerGenerator
      extend T::Sig

      TrustedRuby = RubyGen::TrustedRuby

      class SerializeMethod < RubyGen::Template
        sig {params(name: TrustedRuby, parts: T::Array[PropSerializationLogic]).returns(SerializeMethod).checked(:never)}
        def self.from_parts(name, parts)
          new(
            name: name,
            body: TrustedRuby.join(parts.map(&:generate)),
          )
        end

        sig {override.returns(String).checked(:never)}
        def self.format_string
          <<~RUBY
            def %<name>s(strict)
              h = {}
              %<body>s
              h
            end
          RUBY
        end
      end

      class NilAsserter < RubyGen::Template
        sig {params(prop: Symbol).returns(NilAsserter).checked(:never)}
        def self.for_prop(prop)
          new(prop: RubyGen::SymbolLiteral.new(prop))
        end

        sig {override.returns(String).checked(:never)}
        def self.format_string
          'required_prop_missing_from_serialize(%<prop>s) if strict'
        end
      end

      class PropSerializationLogic < RubyGen::Template
        EMPTY = T.let(TrustedRuby.constant(''), TrustedRuby)
        private_constant :EMPTY

        sig do
          params(
            serialized_form: String,
            accessor_key: Symbol,
            nil_asserter: T.nilable(NilAsserter),
            serialized_val: TrustedRuby,
          )
          .returns(PropSerializationLogic)
          .checked(:never)
        end
        def self.from(serialized_form:, accessor_key:, nil_asserter:, serialized_val:)
          new(
            serialized_form: RubyGen::StringLiteral.new(serialized_form),
            accessor_key: RubyGen::InstanceVar.new(accessor_key),
            nil_asserter: nil_asserter&.generate || EMPTY,
            serialized_val: serialized_val,
          )
        end

        # Don't serialize values that are nil to save space (both the
        # nil value itself and the field name in the serialized BSON
        # document)
        sig {override.returns(String).checked(:never)}
        def self.format_string
          <<~RUBY
            if %<accessor_key>s.nil?
              %<nil_asserter>s
            else
              val = %<accessor_key>s
              h[%<serialized_form>s] = %<serialized_val>s
            end
          RUBY
        end
      end

      sig do
        params(
          name: TrustedRuby,
          props: T::Hash[Symbol, T::Hash[Symbol, T.untyped]],
        )
        .returns(TrustedRuby)
        .checked(:never)
      end
      def self.generate(name, props)
        parts = props.reject {|_, rules| rules[:dont_store]}.map do |prop, rules|
          accessor_key = rules.fetch(:accessor_key)

          PropSerializationLogic.from(
            accessor_key: accessor_key,
            serialized_form: rules.fetch(:serialized_form),
            nil_asserter: rules[:fully_optional] ? nil : NilAsserter.for_prop(prop),
            serialized_val: SerdeTransform.generate(
              T::Utils::Nilable.get_underlying_type_object(rules.fetch(:type_object)),
              SerdeTransform::Mode::SERIALIZE,
            ),
          )
        end

        SerializeMethod.from_parts(name, parts).generate
      end
    end
  end
end

