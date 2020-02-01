# frozen_string_literal: true
# typed: strict

module T::Props
  module Private

    # Generates a specialized `deserialize` implementation for a subclass of
    # T::Props::Serializable, using the facilities in `RubyGen`.
    #
    # The basic idea is that we analyze the props and for each prop, generate
    # the simplest possible logic as a block of Ruby source, so that we don't
    # pay the cost of supporting types like T:::Hash[CustomType, SubstructType]
    # when deserializing a simple Integer. Then we join those together,
    # with a little shared logic to be able to detect when we get input keys
    # that don't match any prop.
    #
    # Each instance of this class is responsible for generating the Ruby source
    # for a single prop; there's a class method which takes a hash of props
    # and generates the full `deserialize` implementation.
    module DeserializerGenerator
      extend T::Sig

      TrustedRuby = RubyGen::TrustedRuby

      class DeserializeMethod < RubyGen::Template
        sig {params(name: TrustedRuby, parts: T::Array[PropDeserializationLogic]).returns(DeserializeMethod).checked(:never)}
        def self.from_parts(name, parts)
          new(
            name: name,
            prop_count: RubyGen::IntegerLiteral.new(parts.size),
            body: TrustedRuby.join(parts.map(&:generate)),
          )
        end

        sig {override.returns(String).checked(:never)}
        def self.format_string
          <<~RUBY
            def %<name>s(hash)
              found = %<prop_count>s
              %<body>s
              found
            end
          RUBY
        end
      end

      class PropDeserializationLogic < RubyGen::Template
        sig do
          params(
            serialized_form: String,
            accessor_key: Symbol,
            nil_handler: T.any(TrustedRuby, RubyGen::TemplateVar),
            serialized_val: TrustedRuby,
          )
          .returns(PropDeserializationLogic)
          .checked(:never)
        end
        def self.from(serialized_form:, accessor_key:, nil_handler:, serialized_val:)
          new(
            serialized_form: RubyGen::StringLiteral.new(serialized_form),
            accessor_key: RubyGen::InstanceVar.new(accessor_key),
            nil_handler: nil_handler,
            serialized_val: serialized_val,
          )
        end

        sig {override.returns(String).checked(:never)}
        def self.format_string
          <<~RUBY
            val = hash[%<serialized_form>s]
            %<accessor_key>s = if val.nil?
              found -= 1 unless hash.key?(%<serialized_form>s)
              %<nil_handler>s
            else
              %<serialized_val>s
            end
          RUBY
        end
      end

      # Generate a method that takes a T::Hash[String, T.untyped] representing
      # serialized props, sets instance variables for each prop found in the
      # input, and returns the count of we props set (which we can use to check
      # for unexpected input keys with minimal effect on the fast path).
      sig do
        params(
          name: TrustedRuby,
          props: T::Hash[Symbol, T::Hash[Symbol, T.untyped]],
          defaults: T::Hash[Symbol, T::Props::Private::ApplyDefault],
        )
        .returns(TrustedRuby)
        .checked(:never)
      end
      def self.generate(name, props, defaults)
        parts = props.reject {|_, rules| rules[:dont_store]}.map do |prop, rules|
          serialized_form = rules.fetch(:serialized_form)

          PropDeserializationLogic.from(
            accessor_key: rules.fetch(:accessor_key),
            serialized_form: serialized_form,
            serialized_val: SerdeTransform.generate(
              T::Utils::Nilable.get_underlying_type_object(rules.fetch(:type_object)),
              SerdeTransform::Mode::DESERIALIZE,
            ),
            nil_handler: DeserializeHandleNil.generate(
              prop: prop,
              serialized_form: serialized_form,
              default: defaults[prop],
              nilable_type: T::Props::Utils.optional_prop?(rules),
              raise_on_nil_write: !!rules[:raise_on_nil_write],
            ),
          )
        end
        DeserializeMethod.from_parts(name, parts).generate
      end
    end
  end
end
