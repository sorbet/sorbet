# frozen_string_literal: true
# typed: strict

module T::Props
  module Private
    # Generates a specialized `initialize` implementation for a subclass of
    # T::Props::Constructor or T::Props::WeakConstructor.
    #
    # The basic idea is that we analyze the props and for each prop, generate
    # the simplest possible logic as a block of Ruby source to set each
    # instance variable to avoid the cost of `instance_variable_set` and other
    # metaprogramming. Then we join those together, with a little shared logic
    # to be able to detect when we get input keys that don't match any prop.
    module InitializerGenerator
      extend T::Sig

      sig do
        params(
          props: T::Hash[Symbol, T::Hash[Symbol, T.untyped]],
          defaults: T::Hash[Symbol, T::Props::Private::ApplyDefault],
          weak: T::Boolean,
        )
          .returns(String)
          .checked(:never)
      end
      def self.generate(props, defaults, weak:)
        parts = props.map do |prop, rules|
          # All of these strings should already be validated (directly or
          # indirectly) in `validate_prop_name`, so we don't bother with a nice
          # error message, but we double check here to prevent a refactoring
          # from introducing a security vulnerability.
          raise unless T::Props::Decorator::SAFE_NAME.match?(prop.to_s)

          ivar_name = rules.fetch(:accessor_key).to_s
          raise unless ivar_name.start_with?('@') && T::Props::Decorator::SAFE_NAME.match?(ivar_name[1..-1])

          # It seems like a bug that this affects the behavior of setters, but
          # some existing code relies on this behavior
          has_explicit_nil_default = rules.key?(:default) && rules.fetch(:default).nil?
          raise_on_nil_write = T::Props::Utils.need_nil_write_check?(rules) && !has_explicit_nil_default

          # Needed to keep behavior unchanged from the non-generated constructor where missing props
          # don't raise if the prop is T.untyped, even though statically sorbet considered that an error.
          raise_on_nil_write = false if rules.fetch(:type_object) == T.untyped

          missing_handler, needs_missing_typecheck = generate_missing_handler(
            prop: prop,
            default: defaults[prop],
            ivar_name: ivar_name,
            raise_on_nil_write: raise_on_nil_write,
            weak: weak,
          )

          typecheck_handler = "#{decorator_fetch(prop, :value_validate_proc)}.call(#{ivar_name})"

          <<~RUBY.strip
            val = hash[#{prop.inspect}]
            if val.nil? && !hash.key?(#{prop.inspect})
              found -= 1
              #{ivar_name} = #{missing_handler}
              #{needs_missing_typecheck ? typecheck_handler : ''}
            else
              #{ivar_name} = val
              #{typecheck_handler}
            end
          RUBY
        end

        <<~RUBY
          def __t_props_generated_initialize(hash)
            found = #{props.size}
            decorator = self.class.decorator

            #{parts.join("\n\n")}

            if found < hash.size
              raise ArgumentError.new("\#{self.class}: Unrecognized properties: \#{(hash.keys - decorator.props.keys).join(', ')}")
            end
          end
        RUBY
      end

      sig do
        params(
          prop: Symbol,
          default: T.nilable(ApplyDefault),
          ivar_name: String,
          raise_on_nil_write: T::Boolean,
          weak: T::Boolean,
        )
          .returns([String, T::Boolean])
          .checked(:never)
      end
      private_class_method def self.generate_missing_handler(
        prop:,
        default:,
        ivar_name:,
        raise_on_nil_write:,
        weak:
      )
        if default
          # FIXME: Ideally we'd check here that the default is actually a valid
          # value for this field, but existing code relies on the fact that we don't.
          #
          # :(
          #
          # The comment above was carried over from `ApplyDefault`.
          [default_value(prop, default), default.is_a?(ApplyDefaultFactory)]
        elsif weak
          [ivar_name, false]
        elsif raise_on_nil_write
          ["raise(ArgumentError.new(\"Missing required prop `#{prop}` for class `\#{self.class.name}`\"))", false]
        else
          ['nil', true]
        end
      end

      sig {params(prop: Symbol, key: Symbol).returns(String)}
      private_class_method def self.decorator_fetch(prop, key)
        "decorator.props.fetch(#{prop.inspect}).fetch(#{key.inspect})"
      end

      sig {params(prop: Symbol, default: ApplyDefault).returns(String)}
      private_class_method def self.default_value(prop, default)
        default_from_props = "decorator.props_with_defaults.fetch(#{prop.inspect}).default"

        case default
        when ApplyPrimitiveDefault
          literal = default.default
          case literal
          when String
            # Always freeze strings here since non-frozen strings use `ApplyComplexDefault`
            "#{literal.inspect}.freeze"
          # `Float` is intentionally left out here because `.inspect` does not produce the correct code
          # representation for non-finite values like `Float::INFINITY` and `Float::NAN` and it's not totally
          # clear that it won't cause issues with floating point precision.
          when Integer, Symbol, TrueClass, FalseClass, NilClass
            literal.inspect
          else
            default_from_props
          end
        when ApplyEmptyArrayDefault
          '[]'
        when ApplyEmptyHashDefault
          '{}'
        else
          default_from_props
        end
      end
    end
  end
end
