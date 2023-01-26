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
          raise_on_missing_key: T::Boolean,
        )
        .returns(String)
        .checked(:never)
      end
      def self.generate(props, raise_on_missing_key:)
        parts = props.map do |prop, rules|
          # All of these strings should already be validated (directly or
          # indirectly) in `validate_prop_name`, so we don't bother with a nice
          # error message, but we double check here to prevent a refactoring
          # from introducing a security vulnerability.
          raise unless T::Props::Decorator::SAFE_NAME.match?(prop.to_s)

          ivar_name = rules.fetch(:accessor_key).to_s
          raise unless ivar_name.start_with?('@') && T::Props::Decorator::SAFE_NAME.match?(ivar_name[1..-1])

          hash_key = prop.inspect
          need_type_check = T::Props::Utils.need_type_check?(rules)

          present_key_value = <<~RUBY.strip
            hash[#{hash_key}]
          RUBY
          present_key_value = wrap_with_typecheck(present_key_value, hash_key) if need_type_check

          if rules.key?(:factory)
            missing_key_value = <<~RUBY.strip
              self.class.class_exec(&decorator.props[#{hash_key}].fetch(:factory))
            RUBY
            missing_key_value = wrap_with_typecheck(missing_key_value, hash_key) if need_type_check
          elsif rules.key?(:default)
            missing_key_value = generate_simple_default(rules.fetch(:default)) || <<~RUBY.strip
              T::Props::Utils.deep_clone_object(decorator.props[#{hash_key}].fetch(:default))
            RUBY
            # FIXME: Ideally we'd check here that the default is actually a valid
            # value for this field, but existing code relies on the fact that we don't.
            #
            # :(
            #
            # missing_key_value = wrap_with_typecheck(missing_key_value, hash_key) if need_type_check
          elsif T::Props::Utils.required_prop?(rules) && raise_on_missing_key
            missing_key_value = <<~RUBY.strip
              raise ArgumentError.new("Missing required prop `#{prop}` for class `\#{self.class.name}`")
            RUBY
          else
            missing_key_value = <<~RUBY.strip
              nil
            RUBY
          end

          <<~RUBY.strip
            #{ivar_name} = if hash.key?(#{hash_key})
              result += 1
              #{present_key_value}
            else
              #{missing_key_value}
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

      private_class_method def self.wrap_with_typecheck(string, hash_key)
        "T::Utils.check_type_recursive!(#{string}, decorator.props[#{hash_key}].fetch(:type_object))"
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
