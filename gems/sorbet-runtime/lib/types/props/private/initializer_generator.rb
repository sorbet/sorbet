# frozen_string_literal: true
# typed: strict

module T::Props
  module Private

    # Generates a specialized `initialize` implementation for a subclass of
    # T::Props::Constructor.
    #
    # The basic idea is that we analyze the props and for each prop, generate
    # the simplest possible logic as a block of Ruby source, so that...
    module InitializerGenerator
      extend T::Sig

      sig do
        params(
          props: T::Hash[Symbol, T::Hash[Symbol, T.untyped]],
          raise_on_missing_required_prop: T::Boolean,
        )
        .returns(String)
        .checked(:never)
      end
      def self.generate(props, raise_on_missing_required_prop:)
        parts = props.map do |prop, rules|
          # All of these strings should already be validated (directly or
          # indirectly) in `validate_prop_name`, so we don't bother with a nice
          # error message, but we double check here to prevent a refactoring
          # from introducing a security vulnerability.
          raise unless T::Props::Decorator::SAFE_NAME.match?(prop.to_s)

          ivar_name = rules.fetch(:accessor_key).to_s
          raise unless ivar_name.start_with?('@') && T::Props::Decorator::SAFE_NAME.match?(ivar_name[1..-1])

          hash_key = prop.inspect

          # It seems like a bug that this affects the behavior of setters, but
          # some existing code relies on this behavior
          has_explicit_nil_default = rules.key?(:default) && rules.fetch(:default).nil?

          need_type_check = T::Props::Utils.need_type_check?(rules)
          need_setter_validate = rules.key?(:setter_validate)
          need_nil_write_check = T::Props::Utils.need_nil_write_check?(rules) && !has_explicit_nil_default

          if rules.key?(:factory)
            need_missing_value_type_check = true
            missing_key_value = <<~RUBY.strip
              self.class.class_exec(&decorator.props[#{hash_key}].fetch(:factory))
            RUBY
          elsif rules.key?(:default)
            need_missing_value_type_check = false
            missing_key_value = generate_simple_default(rules.fetch(:default)) || <<~RUBY.strip
              T::Props::Utils.deep_clone_object(decorator.props[#{hash_key}].fetch(:default))
            RUBY
            # FIXME: Ideally we'd check here that the default is actually a valid
            # value for this field, but existing code relies on the fact that we don't.
            #
            # :(
            #
            # The comment above was carried over from `ApplyDefault`. However, since the "default"
            # is a fixed value, instead of doing the typecheck on initialization we could validate the
            # default here (at load time) like:
            # T::Utils.check_type_recursive!(rules.fetch(:default), rules.fetch(:type_object))
          elsif raise_on_missing_required_prop
            need_missing_value_type_check = true

            if need_nil_write_check
              missing_key_value = <<~RUBY.strip
                raise(ArgumentError.new("Missing required prop `#{prop}` for class `\#{self.class.name}`"))
              RUBY
            end
          else
            need_missing_value_type_check = false
          end

          raise_pretty_error = <<~RUBY.strip
            T::Props::Private::SetterFactory.raise_pretty_error(self.class, #{hash_key}, decorator.props[#{hash_key}].fetch(:type_object), #{ivar_name})
          RUBY

          if need_type_check
            # The `!required_prop?` is a performance optimization since if this is a required prop, nilability would
            # be checked via the typecheck.
            if need_setter_validate || (need_nil_write_check && !T::Props::Utils.required_prop?(rules))
              additional_validations = <<~RUBY
                if #{ivar_name}.nil?
                  #{need_nil_write_check ? raise_pretty_error : ''}
                else
                  #{need_setter_validate ? "decorator.props[#{hash_key}].fetch(:setter_validate).call(#{hash_key}, #{ivar_name})" : ''}
                end
              RUBY
            end

            type_checks = <<~RUBY
              if !decorator.props[#{hash_key}].fetch(:type_object).recursively_valid?(#{ivar_name})
                #{raise_pretty_error}
              end
              #{additional_validations}
            RUBY
          end

          <<~RUBY.strip
            #{ivar_name} = hash[#{hash_key}]
            if #{ivar_name}.nil? && !hash.key?(#{hash_key})
              #{ivar_name} = #{missing_key_value || nil.inspect}
              #{need_missing_value_type_check ? type_checks : ''}
            else
              result += 1
              #{type_checks}
            end
          RUBY
        end

        <<~RUBY
          def __t_props_generated_initialize(hash)
            result = 0
            decorator = self.class.decorator

            #{parts.join("\n\n")}

            if result < hash.size
              raise ArgumentError.new("\#{self.class}: Unrecognized properties: \#{(hash.keys - decorator.props.keys).join(', ')}")
            end
          end
        RUBY
      end

      private_class_method def self.generate_simple_default(default)
        case default
        when String, Integer, Symbol, Float, TrueClass, FalseClass, NilClass
          return default.inspect
        when T::Enum
          # Strips the #<...> off, just leaving the ...
          return default.inspect[2..-2]
        when Array
          if default.empty? && default.class == Array
            return '[]'
          end
        when Hash
          if default.empty? && default.default.nil? && T.unsafe(default).default_proc.nil? && default.class == Hash
            return '{}'
          end
        end
      end
    end
  end
end
